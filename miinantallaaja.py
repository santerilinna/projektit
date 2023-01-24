"""
Miinantallaaja lopputyön koodi.

Santeri Linna
"""
import random
import time
import datetime
import sys
import os
import haravasto

def asetukset():
    """
    Kysyy, millä asetuksilla pelaaja haluaa pelata.
    """
    os.system("cls")
    print("\nTervetuloa miinantallaajaan!\n")
    print("Tekijä: Santeri Linna\n\nOhje: Hiiren vasen aukaisee ruutuja, hiiren oikea asettaa lipun ruutuun.\n")
    print("Valitse 1, jos haluat pelata oletusasetuksilla (15x15 ruudukko, 30 miinaa).")
    print("Valitse 2, jos haluat itse päättää asetukset (minimi koko 3x3, max 25x25).")
    print("Valitse 3, jos haluat katsella tilastoja.")
    print("Valitse 4, jos haluat sulkea pelin.\n")
    try:
        valinta = int(input("Tee valintasi:  "))
        print()
        if valinta == 1:
            paakirjasto["kentan_korkeus"] = 15
            paakirjasto["kentan_leveys"] = 15
            paakirjasto["miinojen_maara"] = 30
        elif valinta == 2:
            paakirjasto["kentan_korkeus"] = int(input("Anna kentän korkeus:  "))
            paakirjasto["kentan_leveys"] = int(input("Anna kentän leveys:  "))
            paakirjasto["miinojen_maara"] = int(input("Anna miinojen määrä:  "))
            if paakirjasto["miinojen_maara"] >= (paakirjasto["kentan_korkeus"] * paakirjasto["kentan_leveys"]):
                paakirjasto["miinojen_maara"] = (paakirjasto["kentan_korkeus"] * paakirjasto["kentan_leveys"])
                print("\nAsetuksillasi jokaisessa ruudussa on miina!")
            if paakirjasto["miinojen_maara"] < 0:
                paakirjasto["miinojen_maara"] = 0
                print("Pelissäsi on 0 miinaa :D")
            if paakirjasto["kentan_korkeus"] > 25 or paakirjasto["kentan_leveys"] > 25:
                print("Liian iso kenttä!")
                input("Enter jatkaaksesi  ")
                asetukset()
            elif paakirjasto["kentan_korkeus"] < 3 or paakirjasto["kentan_leveys"] < 3:
                print("Liian pieni kenttä!")
                input("Enter jatkaaksesi  ")
                asetukset()
        elif valinta == 3:
            katso_tilastot()
        elif valinta == 4:
            sys.exit()
        else:
            print("Laiton valinta!")
            input("Enter jatkaaksesi  ")
            asetukset()
    except ValueError:
        print("Ei ollut luku!")
        input("Enter jatkaaksesi  ")
        asetukset()

def miinoita(vapaat, maara):
    """
    Asettaa kentälle miinat satunnaisiin paikkoihin.
    """
    kentta = paakirjasto["kentta"]
    random.shuffle(vapaat)
    for i in range(maara):
        x1, y1 = vapaat[i]
        kentta[y1][x1] = "x"

def luo_kentta():
    """
    Luo kentän käyttäjän valitsemien parametrien mukaan.
    """
    asetukset()
    kentan_korkeus = paakirjasto["kentan_korkeus"]
    kentan_leveys = paakirjasto["kentan_leveys"]
    miinojen_maara = paakirjasto["miinojen_maara"]
    nakyvakentta = []
    kentta = []
    for _rivi in range(kentan_korkeus):
        kentta.append([])
        nakyvakentta.append([])
        for _sarake in range(kentan_leveys):
            kentta[-1].append(" ")
            nakyvakentta[-1].append(" ")
    paakirjasto["kentta"] = kentta
    paakirjasto["nakyvakentta"] = nakyvakentta
    jaljella = []
    for x in range(kentan_leveys):
        for y in range(kentan_korkeus):
            jaljella.append((x, y))
    miinoita(jaljella, miinojen_maara)
    numerot(kentta)

def numerot(kentta):
    """
    Numeroi kentän kaikki ruudut lähellä olevien miinojen määrän perusteella.
    """
    for rivinumero, rivi in enumerate(kentta):
        for sarakenumero, sarake in enumerate(rivi):
            if sarake != "x":
                kentta[rivinumero][sarakenumero] = str(miinoja_lahella(sarakenumero, rivinumero))
    paakirjasto["kentta"] = kentta

def miinoja_lahella(x, y):
    """
    Laskee ruutua lähellä olevien miinojen määrän.
    """
    kentta = paakirjasto["kentta"]
    naapuri_luku = 0
    for i in range(y-1 if y > 0 else y, y+2 if y < len(kentta)-1 else y+1):
        for j in range(x-1 if x > 0 else x, x+2 if x < len(kentta[0])-1 else x+1):
            if kentta[i][j] == "x":
                naapuri_luku += 1
    return naapuri_luku

def ruudun_naapurit(x, y):
    """
    Kertoo ruudun naapuriruutujen koordinaatit.
    """
    kentta = paakirjasto["kentta"]
    naapuri_lista = []
    for i in range(y-1 if y > 0 else y, y+2 if y < len(kentta)-1 else y+1):
        for j in range(x-1 if x > 0 else x, x+2 if x < len(kentta[0])-1 else x+1):
            naapuri_lista.append((j, i))
    naapuri_lista.remove((x, y))
    return naapuri_lista

def laita_lippu(x, y):
    """
    Asettaa lipun ruutuun tai poistaa lipun.
    """
    if paakirjasto["nakyvakentta"][y][x] == " ":
        paakirjasto["nakyvakentta"][y][x] = "f"
    elif paakirjasto["nakyvakentta"][y][x] == "f":
        paakirjasto["nakyvakentta"][y][x] = " "
    piirra_kentta()

def tutki_ruutu(x, y):
    """
    Tutkii (avaa) klikatun ruudun ja tekee ruudun sisällön perusteella toiminnon.
    """
    paakirjasto["vuorojen_maara"] += 1
    kentta = paakirjasto["kentta"]
    nakyvakentta = paakirjasto["nakyvakentta"]
    if nakyvakentta[y][x] == " ":
        if kentta[y][x] == "0":
            tulvataytto(x, y)
            voitto_tarkistus()
        elif kentta[y][x] == "x":
            havio()
        elif int(kentta[y][x]) > 0:
            nakyvakentta[y][x] = kentta[y][x]
            piirra_kentta()
            voitto_tarkistus()

def tulvataytto(x1, y1):
    """
    Avaa tulvamaisesti kaikki kosketuksessa olevat nollaruudut klikatun nollaruudun ympäriltä.
    Ruutujen avaaminen pysähtyy muihin numeroihin kuin 0 tai alueen reunoihin.
    """
    tarkastetut_ruudut = paakirjasto["tarkastetut_ruudut"]
    kentta = paakirjasto["kentta"]
    nakyvakentta = paakirjasto["nakyvakentta"]
    viereiset_ruudut = ruudun_naapurit(x1, y1)
    if kentta[y1][x1] == "0":
        nakyvakentta[y1][x1] = kentta[y1][x1]
    for x, y in viereiset_ruudut:
        if (x, y) not in tarkastetut_ruudut:
            tarkastetut_ruudut.append((x, y))
            if kentta[y][x] != "x" and nakyvakentta[y][x] != "f":
                nakyvakentta[y][x] = kentta[y][x]
                if kentta[y][x] == "0":
                    tulvataytto(x, y)

def voitto_tarkistus():
    """
    Tarkistaa, täyttyykö voittokriteeri, ja jos täyttyy, tekee voittotoiminnot.
    """
    nakyvakentta = paakirjasto["nakyvakentta"]
    miinojen_maara = paakirjasto["miinojen_maara"]
    avaamattomat_maara = 0
    for rivi in nakyvakentta:
        for sarake in rivi:
            if sarake in (" ", "f"):
                avaamattomat_maara += 1
    if avaamattomat_maara == miinojen_maara:
        paakirjasto["tulos"] = "Voitto"
        print("##### Voitit! Kaikki miinattomat ruudut avattu! #####")
        print(f"##### Aikaa kului {kellon_lopetus():.1f} sekuntia. #####")
        tallennus()

def havio():
    """
    Printtaa häviötekstit ja kutsuu haviamis_asetukset funktiota.
    """
    if paakirjasto["tulos"] != "Voitto":
        paakirjasto["nakyvakentta"] = paakirjasto["kentta"]
        piirra_kentta()
        paakirjasto["tulos"] = "Häviö"
        print("##### Häviö! Räjäytit miinan #####")
        print(f"##### Aikaa kului {kellon_lopetus():.1f} sekuntia. #####")
        tallennus()

def kellon_aloitus():
    """
    Aloittaa kellon, ja tallentaa päivämäärän kirjastoon.
    """
    paakirjasto["alku_aika"] = time.time()
    paakirjasto["paivamaara"] = datetime.datetime.now().strftime("%d-%m-%Y %H:%M:%S")

def kellon_lopetus():
    """
    Pysäyttää kellon ja palauttaa pelin alkuhetkestä kuluneen ajan.
    """
    loppu_aika = time.time()
    kulunut_aika = loppu_aika - paakirjasto["alku_aika"]
    return kulunut_aika

def tallennus():
    """
    Tallentaa pelin tilastot (tulos, aika, päivämäärä yms.) tekstitiedostoon.
    """
    paivamaara = paakirjasto["paivamaara"]
    tulos = paakirjasto["tulos"]
    kesto = kellon_lopetus() / 60
    kentan_korkeus = paakirjasto["kentan_korkeus"]
    kentan_leveys = paakirjasto["kentan_leveys"]
    vuorot = paakirjasto["vuorojen_maara"]
    miinojen_maara = paakirjasto["miinojen_maara"]
    with open("tilastot.txt", "a") as tilastot:
        tilastot.write(f"\n\n\n{paivamaara}\n Tulos: {tulos}\n Kesto: {kesto:.1f} minuuttia \n Kentän korkeus: {kentan_korkeus}")
        tilastot.write(f"\n Kentän leveys: {kentan_leveys}\n Vuorot: {vuorot}\n Miinojen määrä: {miinojen_maara}")

def katso_tilastot():
    """
    Tulostaa tekstitiedoston sisällön.
    """
    with open("tilastot.txt", "r") as tilastot:
        print(tilastot.read())
    print()
    input("Enter jatkaaksesi   ")
    asetukset()

def kasittele_hiiri(x, y, nappi, _muokkaus):
    """
    Tätä funktiota kutsutaan kun käyttäjä klikkaa sovellusikkunaa hiirellä.
    """
    nappaimet = {
    1 : "vasen",
    2 : "rulla", #turha, mutta poistaa erroripläjäyksen jos ruutua klikataan rullan näppäimellä.
    4 : "oikea"
    }
    klikattu_nappi = nappaimet[nappi]
    klikkaus_x = int(x / 40)
    klikkaus_y = int(y / 40)
    if klikkaus_x < len(paakirjasto["nakyvakentta"][0]) and klikkaus_y < len(paakirjasto["nakyvakentta"]):
        if klikattu_nappi == "vasen":
            tutki_ruutu(klikkaus_x, klikkaus_y)
        elif klikattu_nappi == "oikea":
            laita_lippu(klikkaus_x, klikkaus_y)

def piirra_kentta():
    """
    Käsittelijäfunktio, joka piirtää kaksiulotteisena listana kuvatun miinakentän
    ruudut näkyviin peli-ikkunaan. Funktiota kutsutaan aina kun pelimoottori pyytää
    ruudun näkymän päivitystä.
    """
    haravasto.tyhjaa_ikkuna()
    haravasto.aloita_ruutujen_piirto()
    nakyvakentta = paakirjasto["nakyvakentta"]
    for ruudut_korkeus, turha in enumerate(nakyvakentta):
        for ruudut_leveys, merkki in enumerate(turha):
            haravasto.lisaa_piirrettava_ruutu(merkki, ruudut_leveys * 40, ruudut_korkeus * 40)
    haravasto.piirra_ruudut()

paakirjasto = {
    "kentta": [],
    "nakyvakentta": [],
    "alku_aika": None,
    "tarkastetut_ruudut": [],
    "paivamaara": None,
    "kentan_korkeus": None,
    "kentan_leveys": None,
    "miinojen_maara": None,
    "tulos": None,
    "vuorojen_maara": 0
    }

def main():
    """
    Lataa spritet, luo ikkunan ja asettaa piirto- ja hiirikäsittelijät sekä aloittaa pelin kellon.
    """
    luo_kentta()
    haravasto.lataa_kuvat("spritet")
    nakyvakentta = paakirjasto["nakyvakentta"]
    haravasto.luo_ikkuna(len(nakyvakentta[0] * 40), len(nakyvakentta * 40))
    haravasto.aseta_piirto_kasittelija(piirra_kentta)
    haravasto.aseta_hiiri_kasittelija(kasittele_hiiri)
    kellon_aloitus()
    haravasto.aloita()

if __name__ == "__main__":
    main()
