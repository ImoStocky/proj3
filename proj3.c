/*  TITLE
 * Soubor:  proj3.c
 * Datum:   10.12.2004 21:45
 * Autor:   Imrich Štoffa, xstoff02@stud.fit.vutbr.cz
 * Projekt:  projekt č.3  pre predmet IZP
 * Popis:	Program na hladanie priechodu cez bludisko
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

enum task {
    HELP = 1,
    TEST,
    LHAND,
    RHAND,
    SHORT,
    UNK
};

enum error {
    MAZE_FORMAT_ERR=2,
    FILE_ERR,
    MEM_ALLOC_FAILED,
    ARG_VAL_ERR,
    UNK_ARG_ERR
};

const char err_string[][40]={
    "",
    "",
    "Bad file format! Unexpected EOF.\n",
    "Unable to open file.\n",
    "Failed to allocate memory for bitmap\n",
    "Bad argument value. Recheck input.\n",
    "Unknown argumet. Check help.\n"
};

const char *help_str=
{	"""Help:\n"
	"Usage:	--help\n"
	"		--test file\n"
	"		{--lpath|--rpath|--shortest} rows cols file\n"
	"	--help : prints this help page.\n"
	"	--test : verifies that graph representation of maze in file is unoriented.\n"
	"	path alorithms:\n"
	"		--lpath : tries to find exit by always following left wall.\n"
	"		--rpath : like its predecessor but takes right wall.\n"
	"//TODO	--shortest : calculates shortest path though maze.\n"
	"Autor:Imrich Stoffa - xstoff02\n"""
};

enum border {
    LEFT=1,
    RIGHT=2,
    BASE=4
};

typedef struct {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

unsigned char* alloc_map(Map* map)
{
    if((map->cells=(unsigned char*)
                   malloc((size_t)sizeof(char)*map->rows*map->cols))==NULL)
        return NULL;
    else return map->cells;
}

void free_map(Map* map)
{
    free(map->cells);
    map->cells=NULL;
}

unsigned char* get_item(Map *map, int r, int c)
{
    r--;
    c--;
    if(!(0 <= r && r < map->rows)) {
        return NULL;
    }
    if(!(0 <= c && c < map->cols)) {
        return NULL;
    }
    return &map->cells[r*map->cols+c];
}

//Hnusna skereda debilna cas hltajuca funkcia ...uf uz mi je lepsie.
unsigned char mirror_border(unsigned char border)
{
    unsigned char temp = border&(~3);
    border = temp|(((border&1)<<1)|(((border&2)>>1)));
    return border;
}


int next_move(Map *map, int* r, int* c, int border)
{
    switch(border) {
    case LEFT:
        (*c)--;
        break;
    case RIGHT:
        (*c)++;
        break;
    case BASE:
          *r+=((*c)%2==(*r)%2)?-1:1;
        break;
    }
    if((0 < *r && *r <= map->rows) && (0 < *c && *c <= map->cols)) {
        return 1;
    }
    return 0;
}

int read_map(Map *map, FILE* src_f)
{
    //int rows, cols;
    int err=0;
    err=fscanf(src_f,"%d %d",&map->rows,&map->cols);
    if(err==EOF || err < 2 || map->rows <= 0 || map->cols <= 0 ) {
        return MAZE_FORMAT_ERR;
    }//OSETRIT

    if(alloc_map(map)==NULL)
        return MEM_ALLOC_FAILED;

    unsigned int border = 0;

    err=fscanf(src_f,"%u",&border);//OSETRIT
    for(int i = 0 ; i < map->rows*map->cols;
        i++, err=fscanf(src_f,"%u",&border)) {

        if(err != 1)return MAZE_FORMAT_ERR;
        map->cells[i]=(unsigned char)border;
    }
    return EXIT_SUCCESS;
}

bool isborder(Map *map, int r, int c, int border);

bool is_valid(Map *map) //Vlastna funkcia
{
    int nr, nc, x, y, bd;
    unsigned char border_i;
    bool has_exit = 0;
    for(y=1; y<=map->rows; y++) {
        for(x=1 ; x<=map->cols; x++) {
            for(bd = 0; bd < 3; bd++) {
                nr=y;
                nc=x;
                border_i=(1<<bd);
                if(next_move(map,&nr,&nc,border_i)) {
                    if((isborder(map,y,x,border_i)!=isborder(map,nr,nc,mirror_border(border_i)))){
                        return false;
                    }
                }else if(!has_exit){
                //    if((x==1 && border_i==1)||(x==map->cols && border_i==2)||
                //        (y==1 && border_i==4 && x%2==1)||(y==map->rows && border_i==4 && x%2!=y%2)){
                        if(isborder(map,y,x,border_i)==false){
                            has_exit=1;
                        }
                //    }
                }
            }
        }
    }
    return has_exit?true:false;
}

bool isborder(Map *map, int r, int c, int border)
{
    unsigned char * border_val = get_item(map, r, c);

    if(*border_val&border) {
        return true;
    }//else if(border_val==NULL){
    //	return true;
    //}
    return false;
}

int start_border(Map *map, int y, int x, int leftright)
{
    if(x==1&&y==1)return leftright==RIGHT?LEFT:BASE;
    else if(x==1&&y==map->rows){
        if(y%2){ return leftright==RIGHT?BASE:LEFT;
        }else return LEFT;
    } else if(x==map->cols&&y==1){
        if(x%2){ return leftright==RIGHT?BASE:RIGHT;
        }else return RIGHT;
    } else if(x==map->cols&&y==map->rows){
        if(x%2!=y%2){ return leftright==RIGHT?LEFT:BASE;
        }else return RIGHT;
    }
    return 0;
}

int decode_arg(int argc, char **argv, int* x, int* y, char** fi_name, int *task)
{
    *task = UNK;
    if(argc>=2) {
        if(strcmp(argv[1], "--help")==0) {
            *task = HELP;
        } else if(strcmp(argv[1], "--test")==0) {
            if(argc==3) {
                *task = TEST;
            }
        } else if(strcmp(argv[1], "--rpath")==0) {
            *task = RHAND;

        } else if(strcmp(argv[1], "--lpath")==0) {
            *task = LHAND;

        } else if(strcmp(argv[1], "--shortest")==0) {
            *task = SHORT;
        }
    }
    char * check = NULL;
    int conv_ok = 1;

    switch(*task) {

    case RHAND:
    case LHAND:
    case SHORT:

        if(argc==5) {

            *x=strtol(argv[2],&check,10);
            if(*check!='\0')conv_ok=0;
            *y=strtol(argv[3],&check,10);
            if(*check!='\0')conv_ok=0;
            *fi_name=argv[4];
            //something about file
            return conv_ok==0?ARG_VAL_ERR:EXIT_SUCCESS;
        }
        break;

    case TEST:
        *fi_name=argv[2];
        return EXIT_SUCCESS;
        break;
    default:
        if(*task==HELP){
            return argc==2?EXIT_SUCCESS:ARG_VAL_ERR;
        }
        return UNK_ARG_ERR;
        break;
    }
    return EXIT_FAILURE;
}

void hand_wall(Map *map, int r, int c, int leftright){
    int ring[3]={
        LEFT,RIGHT,BASE
    };
    int x = start_border(map,r,c,leftright);
    if(x==0)return;
    int cx = x==4?2:x-1;
    int watch=0;

    x=mirror_border(x);

    do{
        printf("%d,%d\n",r,c);

        watch=0;
        x=mirror_border(x);
        cx = x==4?2:x-1;
        do{
            cx+=((leftright==RIGHT?1:-1)*(r%2==c%2?1:-1));
            cx=(3<=cx)?0:(cx<=-1)?2:cx;
            x=ring[cx];
            if(++watch>3){printf("Nowehere to go !?\n"); return;}
        }while(isborder(map,r,c,x));

    }while(next_move(map, &r, &c, x));
    return;
}


int main(int argc, char **argv)
{
    int r, c;
    char *filename;
    Map maze;
    FILE* source;
    int task;
    int dec= decode_arg(argc, argv, &r, &c, &filename,&task);
    if(dec!=EXIT_SUCCESS) {
        fprintf(stderr,err_string[dec]);
        return dec;
    }else if(task==HELP){
        printf(help_str);
        return EXIT_SUCCESS;
    }

    if((source = fopen(filename,"r"))==NULL) {
        fprintf(stderr,err_string[FILE_ERR]);
        return FILE_ERR;
    }

    if((dec=read_map(&maze, source))!=EXIT_SUCCESS) {
        fclose(source);
        if(maze.cells!=NULL)free_map(&maze);
        fprintf(stderr,err_string[dec]);
        return dec;
    }
    fclose(source);

    int ret_val=EXIT_SUCCESS;
    switch(task) {
    case TEST:
       // printf("--test\n");
        printf("%s\n",is_valid(&maze)?"Valid":"Invalid");
        break;
    case LHAND:
       // printf("--lpath\n");
        hand_wall(&maze,r,c,LEFT);
        break;
    case RHAND:
       // printf("--rpath\n");
        hand_wall(&maze,r,c,RIGHT);
        break;
    case SHORT:
        printf("Unimplemented!\n");
    }
	if(maze.cells!=NULL)free_map(&maze);
    return ret_val;
}
