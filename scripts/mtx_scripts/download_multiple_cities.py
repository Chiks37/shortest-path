import os
from mtx_map_downloader import download_graph, save_graph_data

def main():
    subjects = [
        {
            "place_name": "Петропавловск-Камчатский, Камчатский край, Россия",
            "graph_name": "Petropavlovsk_Kamchatsky",
            "graph_folder": "graphs/UNN/Petropavlovsk_Kamchatsky"
        },
        {
            "place_name": "Сургут, Ханты-Мансийский автономный округ — Югра, Россия",
            "graph_name": "Surgut",
            "graph_folder": "graphs/UNN/Surgut"
        },
        {
            "place_name": "Красноярск, Красноярский край, Россия",
            "graph_name": "Krasnoyarsk",
            "graph_folder": "graphs/UNN/Krasnoyarsk"
        },
        {
            "place_name": "Ростов-на-Дону, Ростовская область, Россия",
            "graph_name": "Rostov_on_Don",
            "graph_folder": "graphs/UNN/Rostov_on_Don"
        },
        {
            "place_name": "Магнитогорск, Челябинская область, Россия",
            "graph_name": "Magnitogorsk",
            "graph_folder": "graphs/UNN/Magnitogorsk"
        },
        {
            "place_name": "Хабаровск, Хабаровский край, Россия",
            "graph_name": "Khabarovsk",
            "graph_folder": "graphs/UNN/Khabarovsk"
        },
        {
            "place_name": "Екатеринбург, Свердловская область, Россия",
            "graph_name": "Yekaterinburg",
            "graph_folder": "graphs/UNN/Yekaterinburg"
        },
        {
            "place_name": "Волгоград, Волгоградская область, Россия",
            "graph_name": "Volgograd",
            "graph_folder": "graphs/UNN/Volgograd"
        },
        {
            "place_name": "Пермь, Пермский край, Россия",
            "graph_name": "Perm",
            "graph_folder": "graphs/UNN/Perm"
        },
        {
            "place_name": "Уфа, Республика Башкортостан, Россия",
            "graph_name": "Ufa",
            "graph_folder": "graphs/UNN/Ufa"
        },

        # {
        #     "place_name": "Ивановская область, Россия",
        #     "graph_name": "Ivanovskaya_Oblast",
        #     "graph_folder": "graphs/UNN/Ivanovskaya_Oblast"
        # },
        # {
        #     "place_name": "Липецкая область, Россия",
        #     "graph_name": "Lipetskaya_Oblast",
        #     "graph_folder": "graphs/UNN/Lipetskaya_Oblast"
        # },
        # {
        #     "place_name": "Тульская область, Россия",
        #     "graph_name": "Tulskaya_Oblast",
        #     "graph_folder": "graphs/UNN/Tulskaya_Oblast"
        # },
        # {
        #     "place_name": "Орловская область, Россия",
        #     "graph_name": "Orlovskaya_Oblast",
        #     "graph_folder": "graphs/UNN/Orlovskaya_Oblast"
        # },
        # {
        #     "place_name": "Тамбовская область, Россия",
        #     "graph_name": "Tambovskaya_Oblast",
        #     "graph_folder": "graphs/UNN/Tambovskaya_Oblast"
        # },
        # {
        #     "place_name": "Брянская область, Россия",
        #     "graph_name": "Bryanskaya_Oblast",
        #     "graph_folder": "graphs/UNN/Bryanskaya_Oblast"
        # },
        # {
        #     "place_name": "Республика Мордовия, Россия",
        #     "graph_name": "Respublika_Mordoviya",
        #     "graph_folder": "graphs/UNN/Respublika_Mordoviya"
        # },
        # {
        #     "place_name": "Ярославская область, Россия",
        #     "graph_name": "Yaroslavskaya_Oblast",
        #     "graph_folder": "graphs/UNN/Yaroslavskaya_Oblast"
        # },
        # {
        #     "place_name": "Рязанская область, Россия",
        #     "graph_name": "Ryazanskaya_Oblast",
        #     "graph_folder": "graphs/UNN/Ryazanskaya_Oblast"
        # },
        # {
        #     "place_name": "Нижегородская область, Россия",
        #     "graph_name": "Nizhegorodskaya_Oblast",
        #     "graph_folder": "graphs/UNN/Nizhegorodskaya_Oblast"
        # }
    ]

    for subject in subjects:
        place_name = subject["place_name"]
        graph_name = subject["graph_name"]
        graph_folder = subject["graph_folder"]

        print(f"=== Обработка города: {place_name} ===")
        G_simple = download_graph(place_name)
        save_graph_data(G_simple, graph_folder, graph_name)

if __name__ == "__main__":
    main()
