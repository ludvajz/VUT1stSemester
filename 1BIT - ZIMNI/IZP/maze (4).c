/**
 * @file maze.c
 * @author Rostislav Ludvik (xludvir00@stud.fit.vutbr.cz)
 * @brief Program, ktery v danem bludisti a jeho vstupu najde pruchod ven. 
 *        Bludiste je ulozeno v textovem souboru ve forme obdelnikove matice celych cisel. 
 *        Cilem programu je vypis souradnic poliček bludiste, pres ktere vede cesta z vchodu bludiste do jeho vychodu
 * 
 * @date 2023-11-28
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// definice globalni konstanty pro hranice trojuhelniku
const int LEFT = 1;
const int RIGHT = 2;
const int BOTTOM_OR_TOP = 4;

// struktura bludiste ze zadani
typedef struct {
  int rows;
  int cols;
  unsigned char *cells;
} Map;

// prototypy fuknci ze zadani
bool isborder(Map *map, int r, int c, int border); // 2. podukol
int start_border(Map *map, int r, int c, int leftright); // 3 podukol

// vlastni funkce
bool hasbottomborder(int r, int c);
int following_border(int r, int c, int current_border, int leftright);
int following_crossable_border(Map *map, int r, int c, int current_border, int leftright);
int path(Map *map, long int *r, long int *c, int *border, int leftright);


// spusteni programu
int main(int argc, char *argv[]){

    /**
     * @brief spusti se kdyz arg je --help, zobrazi napovedu
     * 
     */
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
      printf("Pouziti: %s [Moznosti]\n", argv[0]);
      printf("Moznosti:\n");
      printf("  --help    Zobrazi tuto zpravu\n");
      printf("  --test    Otestuje zda je bludiste v souboru validni\n");
      printf("  --rpath R C    Projde pole pravidel prave ruky\n");
      printf("  --lpath R C    Projde pole pravidel leve ruky\n");
      return 0;
    }

    FILE *file;
    if (argc == 3 && strcmp(argv[1], "--test") == 0)
    {
      file = fopen(argv[2], "r");     // otevre soubor k read only
    }
    if (argc == 5)
    {
      file = fopen(argv[4], "r");     // otevre soubor k read only
    }

    if (file == NULL) {                 // kontrola otevreni souboru
      fprintf(stderr, "Soubor nebyl otevren\n");
      return 1; 
    }

    Map map;    // deklarace bludiste
    fscanf(file, "%d %d\n", &map.rows, &map.cols); // nacteni R C ze souboru do struktury map
    map.cells = malloc(map.rows * map.cols * sizeof(unsigned char)); // alokace pameti pro bunky bludiste

    if (map.cells == NULL) // kontrola alokace
    {
      fprintf(stderr, "Bunky se nealokovaly, program končí\n"); 
      return 1;
    }

    for (int i=0; i < (map.rows * map.cols); i++){ // inicializace bunek bludiste
      map.cells[i] = fgetc(file);
      fgetc(file);
    }

    fclose(file);  // zavreni souboru po dokonceni cteni

    /**
     * @brief Otestuje Validitu bludiste
     * 
     */
    if (argc == 3 && strcmp(argv[1], "--test") == 0){

      for (int i = 0; i < (map.rows * map.cols); i++)
      {
        if ((map.cells[i] <= '0') && (map.cells[i] >= '7')) // bunky bludiste maji hodnoty od 0 do 7
        {
          printf("Invalid\n");
          free(map.cells);
          return 0;
        }
      }
      
      for (int r = 1; r <= map.rows; r++)
      {
        for (int c = 1; c <= map.cols; c++)
        {
          if (c < map.cols && isborder(&map, r, c, RIGHT) != isborder(&map, r, c + 1, LEFT)){ // 2 sousedici bunky na radku nemaji stejnou hranici
            printf("Invalid\n");
            free(map.cells);
            return 0;
          }
          if (hasbottomborder(r, c))
          {
            if (r < map.rows && isborder(&map, r, c, BOTTOM_OR_TOP) != isborder(&map, r + 1, c, BOTTOM_OR_TOP)){ //2 sousedici bunky na sloupci nemaji stejnou hranici
              printf("Invalid\n");
              free(map.cells);
              return 0;
            }
          }
        }
      }
      printf("Valid\n");
      free(map.cells); // uvolni alokovane bunky, zabrani memory leak
      return 0;       
    }

    if (argc == 5)
    { 
      int leftright; //deklarace promenne pro pravidlo ruky
      char *err; //pole pro chybne znaky strol
        
      long r = strtol(argv[2], &err, 10); //prevedeni argumentu na long int, chybne znaky se ulozi do pole err
      if (err != NULL && strlen(err) > 0){
        fprintf(stderr, "Treti argument urcen pro radek neni cislo\n"); 
        return 1;
      }

      long c = strtol(argv[3], &err, 10); //prevedeni argumentu na long int, chybne znaky se ulozi do pole err
      if (err != NULL && strlen(err) > 0){
        fprintf(stderr, "Ctvrty argument urcen pro sloupec neni cislo\n");
        return 1;
      }
        
      if (strcmp("--rpath", argv[1]) == 0) //pravidlo prave ruky
        {leftright = RIGHT;}
      else if (strcmp("--lpath", argv[1]) == 0){ //pravidlo leve ruky
        leftright = LEFT;
      }
      else{
        printf("Spatny paty argument, neni --rpath ani --lpath\n"); 
        return 1;
      }

      int border = start_border(&map, r, c, leftright); // zvoleni steny ktere se mame chytit rukou
      border = following_crossable_border(&map, r, c, border, leftright);
      printf("%ld,%ld\n", r, c); //vypis pocatecni pozice

      while (path(&map, &r, &c, &border, leftright)) // dokud mame moznou cestu, vypisujeme souradnice
      {
        printf("%ld,%ld\n", r, c);
      }

      free(map.cells); // uvolni alokovane bunky bludiste, zabrani memory leak  
      return 0;      
    }
  return 0;
}

/**
 * @brief Vraci true kdyz na dotazovane hranici lezi nepruchozi stena, jinak false
 * 
 * @param map ukazatel na strukturu mapy
 * @param r souradnice 
 * @param c souradnice
 * @param border dotazovana hranice
 */
bool isborder(Map *map, int r, int c, int border)
{
  if (((map->cells[(r - 1) * map->cols + c - 1] - '0') & border) != 0 )
  {
    return true;
  }
  else{
    return false;
  }
}

/**
 * @brief Vraci nasledujici hranici pri vstupu do bludiste
 * 
 * @param map ukazatel na strukturu mapy
 * @param r souradnice
 * @param c souradnice
 * @param leftright pravidlo ruky
 */
int start_border(Map *map, int r, int c, int leftright)
{
  // priorita podle zadani: 1,2,5,6,3,4
  // 1 
  if (c == 1 && r % 2 == 1){ // prava pri vstupu zleva do bludiste na lichem radku
    if (leftright == RIGHT)
    {
      return RIGHT;
    }
    if (leftright == LEFT)
    {
      return BOTTOM_OR_TOP;
    }
  }
  // 2 
  if (c == 1 && r % 2 == 0){ // dolni pri vstupu zleva do bludiste na sudem radku
    if (leftright == RIGHT)
    {
      return BOTTOM_OR_TOP;
    }
    if (leftright == LEFT)
    {
      return RIGHT;
    }
  }
  // 5
  if (c == map->cols && !hasbottomborder(r, c)){ // horni pri vstupu zprava do bludiste, pokud ma policko horni hranici
    if (leftright == RIGHT)
    {
      return BOTTOM_OR_TOP;
    }
    if (leftright == LEFT)
    {
      return LEFT;
    }
  }
  // 6
  if (c == map->cols && hasbottomborder(r, c)){ // leva pri vstupu zprava do bludiste, pokud ma policko dolni hranici
    if (leftright == RIGHT)
    {
      return LEFT;
    }
    if (leftright == LEFT)
    {
      return BOTTOM_OR_TOP;
    }
  }
  // 3
  if (r == 1){ // leva pri vstupu shora do bludiste
    if (leftright == RIGHT)
    {
      return LEFT;
    }
    if (leftright == LEFT)
    {
      return RIGHT;
    }
  }
  // 4
  if (r == map->rows){ // prava pri vstupu zespodu do bludiste
    if (leftright == RIGHT)
    {
      return RIGHT;
    }
    if (leftright == LEFT)
    {
      return LEFT;
    }
  }   

  return 0;
}

/**
 * @brief Vraci true jestli ma pole spodni hranici, jinak false
 * 
 * @param r souradnice
 * @param c souradnice
 */
bool hasbottomborder(int r, int c)
{
  if ((r % 2 == 0 && c % 2 == 1) || (r % 2 == 1 && c % 2 == 0))
  {
    return true;
  }
  else{
    return false;
  }
}

/**
 * @brief Vraci nasledujici hranici podle pravidla ruky pro kontrolu funkci following_crossable_border
 * 
 * @param r souradnice radku
 * @param c souradnice sloupce
 * @param current_border aktualni hranice
 * @param leftright pravidlo ruky
 * @return int 
 */
int following_border(int r, int c, int current_border, int leftright)
{
  if (hasbottomborder(r, c))
  {
    if (current_border == BOTTOM_OR_TOP){
      if (leftright == LEFT){
        return LEFT;
      }
      else{
        return RIGHT;
      }
    }
    if (current_border == RIGHT){
      if (leftright == LEFT){
        return BOTTOM_OR_TOP;
      }
      else{
        return LEFT;
      }
    }
    else{
      if (leftright == LEFT){
        return RIGHT;
      }
      else{
        return BOTTOM_OR_TOP;
      }
    }
  }
  else
  {
    if (current_border == BOTTOM_OR_TOP){
      if (leftright == RIGHT){
        return LEFT;
      }
      else{
        return RIGHT;
      }
    }
    if (current_border == RIGHT){
      if (leftright == RIGHT){
        return BOTTOM_OR_TOP;
      }
      else{
        return LEFT;
      }
    }
    else{
      if (leftright == RIGHT){
        return RIGHT;
      }
      else{
        return BOTTOM_OR_TOP;
      }
    }
  }
}

/**
 * @brief Vraci dalsi dranici podle pravidla ruky ktera lze prekrocit
 * 
 * @param map ukazatel na strukturu mapy
 * @param r souradnice radku
 * @param c souradnice sloupce
 * @param current_border aktualni hranice
 * @param leftright pravidlo ruky
 */
int following_crossable_border(Map *map, int r, int c, int current_border, int leftright)
{
  int border = current_border;
  if (isborder(map, r, c, border))
  {
    border = following_border(r, c, border, leftright);
    if (isborder(map, r, c, border))
    {
      border = following_border(r, c, border, leftright);
    }
  }

  return border;
}

/**
 * @brief vraci 1 kdyz existuje cesta pro posun v bludisti, jinak 0
 * 
 * @param map ukazatel na strukturu mapy
 * @param r souradnice radku
 * @param c souradnice sloupce
 * @param current_border aktualni hranice
 * @param leftright pravidlo ruky
 */
int path(Map *map, long int *r, long int *c, int *border, int leftright)
{
  if (*border == LEFT)
  {
    *c = *c - 1;
    if (*c < 1){
      return 0;
    }
    if (hasbottomborder(*r, *c)){
      if (leftright == LEFT)
      {
        *border = BOTTOM_OR_TOP;
      }
      else{
        *border = LEFT;
      } 
    }
    else{
      if (leftright == LEFT)
      {
        *border = LEFT;
      }
      else{
        *border = BOTTOM_OR_TOP;
      }
      
    }
  }
  else if (*border == RIGHT)
  {
    *c = *c + 1;
    if (*c > map->cols){
      return 0;
    }
    if (hasbottomborder(*r, *c)){
      if (leftright == LEFT)
      {
        *border = RIGHT;
      }
      else{
        *border = BOTTOM_OR_TOP;
      }
    }
    else{
      if (leftright == LEFT)
      {
        *border = BOTTOM_OR_TOP;
      }
      else{
        *border = RIGHT;
      }
    }
  }
  else if (*border == BOTTOM_OR_TOP)
  {
    if (hasbottomborder(*r, *c))
    {
      *r = *r + 1;
      if (*r > map->rows){
        return 0;
      }  
      if (leftright == LEFT){
        *border = RIGHT;
      }
      else{
        *border = LEFT;
      }
    }
    else{
      *r = *r - 1;
      if (*r < 1)
        return 0;
      if (leftright == LEFT)
      {
        *border = LEFT;
      }
      else{
        *border = RIGHT;
      }
    }
  }

  *border = following_crossable_border(map, *r, *c, *border, leftright);
  return 1;
}