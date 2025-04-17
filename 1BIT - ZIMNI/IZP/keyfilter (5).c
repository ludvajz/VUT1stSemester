/**
 * @file keyfilter.c
 * @author Rostislav Ludvik (xludvir00@stud.fit.vutbr.cz)
 * @brief   Emulace algoritmu vypoctu povolenych a zakazanch klaves na virtualni klavesnici navigace. 
 *          Vstupem programu jsou data znamych adres a uzivatelem zadany retezec. 
 *          Vystup programu bude obsahovat znaky, ktere maji byt na klavesnici povoleny.
 * @version 0.1
 * @date 2023-10-24
*/

#include<stdio.h>   // vyuziti stdin a stdout
#include<string.h>  // vyuziti funkce strncat
#include<ctype.h>   // vyuziti funkce toupper

/**
 * @brief funkce s vedlejsim efektem, dosazeni case insensitive
 *          od indexu 0 nahrazuje v retezci mala pismena velkymi
 * 
 * @param str vystup je retezec bez malych pismen
 */
void toUpper(char str[]){
    for (int i = 0; str[i] != '\0'; i++)
    {
        str[i] = toupper(str[i]);
    }
}

/**
 * @brief funkce se spusti kdyz byl program spusnen bez argumentu
 *          funkce s vedlejsim efektem, upravuje parametry
 *          pri kazdem cyklu nacte ze zasobniku retezec, v nem prevede mala pismena na velka
 *          a prida prvni znak do retezce 'enable'
 * 
 * @param buffer pole znaku do ktereho se nacte dany retezec ze souboru adresy.txt
 * @param enable pole znaku do ktereho se napisuji povolene znaky
 */
void firtsLetters(char buffer[], char enable[]){
    while (fgets(buffer, 102, stdin) != NULL){
        toUpper(buffer);
        strncat(enable, &buffer[0], 1);
    }
}

/**
 * @brief funkce s vedlejsim efektem, upravuje parametry
 * 
 * @param poleArg vstup - retezec argumentu  
 * @param buffer vstup - retezec nacteny z adresy.txt
 * @param enable vstup - prazdny retezec, vystup - retezec povolenych znaku
 * @param enableFound vstup - prazdny retezec, vystup - retezec 
 * @param found vstupni hodnota 0, kdyz je vstupni argument znama adresa, zmeni se na hodnotu 1, jinak zustava 0
 */
void porovnat2(char poleArg[], char buffer[], char enable[], char enableFound[], int *found){

    int i = 0; // aktualni index pole a bufferu, na vystupu oznacuje index kde provnavani skoncilo
    
    while (poleArg[i] == buffer[i]) // dokud se znaky na stejnych indexech rovnaji, inkrementujeme index
    {
        i++;
    }
    if (poleArg[i] == '\0' && buffer[i] == '\n') // kdyz se argument rovna s retezcem ze zasobniku, jedna se o znamou adresu
    {
        printf("Found: %s", buffer); 
        *found = 1;  // kdyz bude found = 1 tak netiskne enable
    }
    else if (poleArg[i] == '\0' && buffer[i] != '\n') // kdyz se vsechny znaky argumentu rovnaji s retezcem ze zasobniku ale zasobnik je delsi, ulozi dalsi znak do retezce enable
    {
        strncat(enable, &buffer[i], 1);
        strcat(enableFound, buffer);
    }
}

/**
 * @brief funkce s vedlejsim efektem ktera seradi znaky ulozene v retezci enable podle abecedy do noveho pole 'enableSorted' 
 * 
 * @param enable vstup - povolene znaky ktere nemusi byt abecedne serazeny
 * @param enableSorted vystup - abecedne serareny retezec
 */
void enableSort(char enable[], char enableSorted[]){
    int enableSize = strlen(enable);

    // kopiruje pole do noveho pole
    strcpy(enableSorted, enable);

    // seradi nove podle podle hodnot z Ascii tabulky
    for (int i = 0; i < enableSize - 1; i++) {
        for (int j = i + 1; j < enableSize; j++) {
            if (enableSorted[i] > enableSorted[j]) {
                // prohodi znaky pomoci docasneho pole tak aby byly serazene
                char temp = enableSorted[i];
                enableSorted[i] = enableSorted[j];
                enableSorted[j] = temp;
            }
        }
    }
}

/**
 * @brief funkce s vedlejsim efektem ktera eliminuje duplikovane znaky v retezci
 * 
 * @param enableSorted vstup - retezec ktery muze obsahovat stejny znak vicekrat, vystup - retezec, ktery obsahuje urcity znak jen jednou
 */
void removeDuplicate(char enableSorted[]) {

    int length = strlen(enableSorted);
    int nextIndex = 1;

    for (int i = 1; i < length; i++) {
        if (enableSorted[i] != enableSorted[i - 1]) {
            enableSorted[nextIndex] = enableSorted[i];
            nextIndex++;
        }
    }

    enableSorted[nextIndex] = '\0'; // ukonceni retezce
}

// zacatek programu
int main(int argc, char* argv[]){
    
    char buffer[102]; // deklarace bufferu do ktereho se nacitaji adresy z txt file, predpodladame adresu o maximalni delce 100 znaku
    
    char enable[102] = ""; // deklarace prazdneho pole povolenych znaku do ktereho budeme nacitat povolene znaky
    
    char enableFound[102] = ""; // deklarace prazdneho pole znaku do ktereho budeme ukladat adresu jejiz prefix je unikatni

    int found = 0; // deklarace hodnoty podle ktere budeme urcovat zda uzivatel napsal na stdin adresu z adresare

    char enableSorted[102]; 

    /**
     * @brief spusti se kdyz se program spustil s vice nez 2 argumenty
     *  neocekavane chovani, vypise chybu, vrati hodnotu 1 a skonci program
    */
    if (argc > 2)
    {
        fprintf(stderr, "Too many arguments");
        return 1;
    }
    
    /**
     * @brief spusti se kdyz se program spustil s 0 argumenty
     *  do retezce enable se ulozi jen prvni pismena vsech mest, seradi se, vyfiltruji se, vypisou se
     *  kdyz je retezec enable prazdny => soubor adres je prazdny
    */
    if(argc < 2 || strlen(argv[1]) == 0){
        firtsLetters(buffer, enable);
        enableSort(enable, enableSorted);
        removeDuplicate(enableSorted);
        if (strlen(enable) == 0)
        {
            fprintf(stderr, "Empty file of adresses\n");
            return 1;
        }
        printf("Enable: %s\n", enableSorted);
        return 0;
    } 

    // argument prevedeme na pole se kterym budeme dal pracovat, prekpokladame ze je argument dlouhy max. 100 znaku
    char poleArg[102];
    for(int j=0; j<102; j++){
        poleArg[j] = argv[1][j];
    }

    toUpper(poleArg); // prevedeni pole z argumentu na velka pismena

    /**
     * @brief fgets nacita do bufferu 102 znaku, provedou se funkce, opakuje se dokud je do bufferu co nacitat
     */
    while (fgets(buffer, 102, stdin) != NULL){
        toUpper(buffer);
        porovnat2(poleArg, buffer, enable, enableFound, &found);
    }

    // kdyz je povoleno vice nez 2 znaky tak vypise Enable + vytridene pole povolenych znaku bez duplikatu 
    if (enable[0] != '\0' && enable[1] != '\0')
    {
        enableSort(enable, enableSorted);
        removeDuplicate(enableSorted);
        printf("Enable: %s\n", enableSorted);
    }
    
    // kdyz neni povoleny zadny znak a zaroven uzivatel nenapsal adresu z adresare, nebyla nalezena zadna shoda
    if (enable[0] == '\0' && found != 1) 
    {
        printf("Not found\n");
    }
    
    // kdyz uzivatel zada do stdin unikatni prefix adresy z adresare, vypise se adresa
    if (enable[0] != '\0' && enable[1] == '\0' && found == 0){
        printf("Found: %s", enableFound);
    }

    return 0;
}