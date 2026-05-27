import argparse
import time
from pathlib import Path

import numpy as np
import scipy.io
import scipy.sparse
import scipy.sparse.csgraph
import scipy.sparse.linalg


def load_adjacency(mtx_path: Path) -> scipy.sparse.csr_matrix:
    mat = scipy.io.mmread(str(mtx_path)).tocsr()
    if mat.shape[0] != mat.shape[1]:
        raise ValueError(f"{mtx_path}: expected square matrix, got {mat.shape}")
    sym = mat + mat.T
    sym.data[:] = 1.0
    sym = sym.tocsr()
    sym.eliminate_zeros()
    sym.setdiag(0)
    sym.eliminate_zeros()
    return sym


def spectral_embedding(
    adj: scipy.sparse.csr_matrix, seed: int, tol: float, maxiter: int
) -> np.ndarray:
    n = adj.shape[0]
    laplacian = scipy.sparse.csgraph.laplacian(adj, normed=False).tocsr()

    t0 = time.time()
    if n < 2000:
        eigvals, eigvecs = np.linalg.eigh(laplacian.toarray())
        print(f"dense eigh: eigvals[:4]={eigvals[:4]}, elapsed={time.time() - t0:.1f}s")
        return eigvecs[:, 1:3]

    rng = np.random.default_rng(seed)
    init = rng.standard_normal((n, 3))

    diag = laplacian.diagonal()
    diag = np.where(diag > 0, diag, 1.0)
    precond = scipy.sparse.diags(1.0 / diag).tocsr()

    eigvals, eigvecs = scipy.sparse.linalg.lobpcg(
        laplacian,
        init,
        M=precond,
        largest=False,
        tol=tol,
        maxiter=maxiter,
        verbosityLevel=1,
    )
    print(f"lobpcg: eigvals={eigvals}, elapsed={time.time() - t0:.1f}s")

    order = np.argsort(eigvals)
    return eigvecs[:, order[1:3]]


def normalize_coords(coords: np.ndarray, scale: float) -> np.ndarray:
    lo = coords.min(axis=0)
    hi = coords.max(axis=0)
    span = np.where(hi - lo > 0, hi - lo, 1.0)
    return (coords - lo) / span * scale


def compute_weights(
    coords: np.ndarray, adj: scipy.sparse.csr_matrix
) -> scipy.sparse.coo_matrix:
    coo = scipy.sparse.tril(adj, k=-1).tocoo()
    rows = coo.row
    cols = coo.col
    dx = coords[rows, 0] - coords[cols, 0]
    dy = coords[rows, 1] - coords[cols, 1]
    weights = np.hypot(dx, dy)
    return scipy.sparse.coo_matrix(
        (weights, (rows, cols)), shape=adj.shape
    )


def write_nodes_mapping(out_path: Path, coords: np.ndarray) -> None:
    with out_path.open("w") as f:
        for i, (x, y) in enumerate(coords):
            f.write(f"{i + 1}\t{i} {i} 0 {x:.10f} {y:.10f} 0\n")


def main() -> None:
    parser = argparse.ArgumentParser(
        description=(
            "Build a 2D spectral layout of a graph and rewrite edge weights as "
            "Euclidean distances in that layout. Produces a new .mtx and a "
            "*_nodes_mapping.txt consistent with the AStarG heuristic."
        ),
        epilog=(
            "Examples:\n"
            "  # California road network (default output: roadNet-CA-weighted_spectral.mtx)\n"
            "  python spectral_geo_layout.py graphs/roadNet-CA-weighted.mtx\n"
            "\n"
            "  # Explicit output paths\n"
            "  python spectral_geo_layout.py graphs/roadNet-CA-weighted.mtx \\\n"
            "      --output-mtx graphs/roadNet-CA-geo.mtx \\\n"
            "      --output-nodes graphs/roadNet-CA-geo_nodes_mapping.txt\n"
            "\n"
            "  # Larger coordinate box (bigger edge weights, same A* correctness)\n"
            "  python spectral_geo_layout.py graphs/roadNet-CA-weighted.mtx --scale 100000\n"
            "\n"
            "  # Relax solver for very large graphs if lobpcg hits maxiter\n"
            "  python spectral_geo_layout.py graphs/roadNet-CA-weighted.mtx --tol 1e-4 --maxiter 1000\n"
            "\n"
            "After generation, run:\n"
            "  ./bin/custom_test_gapbs graphs/roadNet-CA-weighted_spectral.mtx"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument("mtx", help="Input .mtx graph file.")
    parser.add_argument(
        "--output-mtx",
        default=None,
        help="Output .mtx path. Default: <input>_spectral.mtx",
    )
    parser.add_argument(
        "--output-nodes",
        default=None,
        help="Output nodes mapping path. Default: <input>_spectral_nodes_mapping.txt",
    )
    parser.add_argument(
        "--scale",
        type=float,
        default=1000.0,
        help="Coordinate box side. Edge weights and heuristic scale proportionally.",
    )
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument(
        "--tol", type=float, default=1e-5, help="lobpcg tolerance."
    )
    parser.add_argument(
        "--maxiter", type=int, default=500, help="lobpcg max iterations."
    )
    args = parser.parse_args()

    mtx_path = Path(args.mtx)
    out_mtx = (
        Path(args.output_mtx)
        if args.output_mtx
        else mtx_path.with_name(mtx_path.stem + "_spectral.mtx")
    )
    out_nodes = (
        Path(args.output_nodes)
        if args.output_nodes
        else out_mtx.with_name(out_mtx.stem + "_nodes_mapping.txt")
    )

    print(f"Loading {mtx_path}...")
    adj = load_adjacency(mtx_path)
    print(f"  vertices={adj.shape[0]}, undirected_edges={adj.nnz // 2}")

    print("Computing spectral embedding (this may take several minutes)...")
    coords = spectral_embedding(adj, args.seed, args.tol, args.maxiter)
    coords = normalize_coords(coords, args.scale)

    print("Recomputing edge weights as Euclidean distances...")
    weighted = compute_weights(coords, adj)

    out_mtx.parent.mkdir(parents=True, exist_ok=True)
    print(f"Writing {out_mtx}...")
    scipy.io.mmwrite(
        str(out_mtx),
        weighted,
        field="real",
        symmetry="symmetric",
    )

    print(f"Writing {out_nodes}...")
    write_nodes_mapping(out_nodes, coords)

    print(
        f"Done. Run with: ./bin/custom_test_gapbs {out_mtx}\n"
        f"(the *_nodes_mapping.txt is picked up automatically)."
    )


if __name__ == "__main__":
    main()
