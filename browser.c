#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <menu.h>
#include <ctype.h>

/*Strategia de lucru principala este deshiderea unor ferestre suprapuse,
incepand cu numele motorului de cautare, urmand bara de cautare, meniul
si continutul siteului. Daca dam back in vreo instanta, se va crea 
o alta fereastra, dar cu aceleasi elemente*/
/*Obs: am impartit cateva anteturi pe 2 randuri pt a nu depasi
80 de caractere pe linie*/

//enumeratia "background" de culori pt text si fundal
typedef enum {white,black,red,green,blue,yellow} background;

//structura in sine
//url, lungime, titlu, accesari, checksum-> self-explanatory
//cod- contine tot html-ul siteului
//continut- contine doar continutul unui site (intre p-uri)
//text-culoarea textului; fundal-culoarea fundalului;
//nume_fis- contine numele fisierului ce are detaliile siteului
//valid-> posibilitatea de a fi incarcat in site_cu_cuvinte,
//pentru a evita duplicatele (indiferent daca e simplu sau avansat)
//valid=0 -> a fost deja incarcat
//valid=1 -> nu a fost incarcat, dar poate fi
typedef struct
{
    char url[51],titlu[51],*cod,*continut,nume_fis[31];
    int accesari,checksum,lungime,valid;
    background text,fundal;
}SITE;

//alocam culori textului si fundalului
//toate culorile au a3a litera diferita
//point 1 reprezinta locul unde ne aflam in tag
void switch_culori(char *point1, char *point3, SITE *site)
{
    switch(point1[0])
    {
        //daca incepe cu c "color"
        case 'c':
        {
            point3=strstr(point1,":");
            switch(point3[3])
            {
                case 'd':
                    site->text=red;
                    break;
                case 'a':
                    site->text=black;
                    break;
                case 'i':
                    site->text=white;
                    break;
                case 'e':
                    site->text=green;
                    break;
                case 'u':
                    site->text=blue;
                    break;
                case 'l':
                    site->text=yellow;
                    break;
                default:
                    site->text=black;
            }
            break;
        }
        //daca incepe cu b "background"
        case 'b':
        {
            point3=strstr(point1,":");
            switch(point3[3])
            {
                case 'd':
                    site->fundal=red;
                    break;
                case 'a':
                    site->fundal=black;
                    break;
                case 'i':
                    site->fundal=white;
                    break;
                case 'e':
                    site->fundal=green;
                    break;
                case 'u':
                    site->fundal=blue;
                    break;
                case 'l':
                    site->fundal=yellow;
                    break;
                default:
                    site->fundal=white;
            }
            break;
        }
    }
}

//parsarea titlului, ne miscam intre pointeri
void titlul(SITE *site1)
{
    char *point1,*point2;
    point1=strstr(site1->cod,"<title>");
    point2=strstr(site1->cod,"</title>");
    point1+=7; //trecem imediat dupa <title>
    int j=0;
    while(point1!=point2)
    {
        site1->titlu[j++]=(point1[0]);
        point1++;
    }
    site1->titlu[j]='\0';
}

//parsarea continutului, tot cu miscare de pointeri
void continutul(SITE *site1)
{
    char *point1,*point2,*point3;
    int j;
    point1=strstr(site1->cod,"<p>");
    point2=strstr(site1->cod,"</p>");
    if(point1)
    {
        //daca tagul este <p>
        site1->continut=(char *)malloc((point2-point1-3+1)*sizeof(char));
        point1+=3;
        j=0;
        while(point1!=point2)
        {
            site1->continut[j++]=(point1[0]);
            point1++;
        }
        site1->continut[j]='\0';
    }
    else
    {
        //daca tagul cuprinde cel putin o culoare
        point1=strstr(site1->cod,"<p style=");
        //pentru continut exclusiv
        point3=strstr(point1,">");
        site1->continut=(char *)malloc((point2-point3-1+1)*sizeof(char));
        point3++;
        j=0;
        while(point3!=point2)
        {
            site1->continut[j++]=(point3[0]);
            point3++;
        }
        site1->continut[j]='\0';
        //pentru culori
        point1+=10;
        switch_culori(point1,point3,site1);
        //aici verificam daca mai exista si al 2lea tag de culori
        point3=strstr(point1,";");
        point1=point3+1;
        switch_culori(point1,point3,site1);
    }
}

//functie pentru afisarea continutului unui site
void enter(SITE final)
{
    //creare fereastra noua
    WINDOW *win=newwin(0,0,0,0);
    wrefresh(win);
    box(win,0,0);
    wbkgd(win,COLOR_PAIR(1));
    wattron(win,COLOR_PAIR(1));
    curs_set(0); //cursor invizibil
    int a,b,y=1,x=1,c;
    //white=7; blue=4; green=2; black=0; yellow=3; red=1;
    //detectare culoare fundal
    switch (final.fundal)
    {
        case white:
            a=7;
            break;
        case blue:
            a=4;
            break;
        case green:
            a=2;
            break;
        case black:
            a=0;
            break;
        case yellow:
            a=3;
            break;
        case red:
            a=1;
            break;
        default:
            break;
    }
    //detectare culoare text
    switch (final.text)
    {
        case white:
            b=7;
            break;
        case blue:
            b=4;
            break;
        case green:
            b=2;
            break;
        case black:
            b=0;
            break;
        case yellow:
            b=3;
            break;
        case red:
            b=1;
            break;
        default:
            break;
    }
    //generare pereche de culori si colorare window
    init_pair(2,b,a);
    wattron(win,COLOR_PAIR(2));
    wbkgd(win,COLOR_PAIR(2));
    //text boldat pe mijloc
    wattron(win,A_BOLD);
    x=(getmaxx(win))/2-strlen(final.titlu)/2;
    mvwprintw(win,y++,x,final.titlu);
    wattroff(win,A_BOLD);
    //afisare continut
    mvwprintw(win,y,5,final.continut);
    //afisare legenda
    x=(getmaxx(win))-20;
    y=(getmaxy(win))-5;
    mvwprintw(win, y++, x, "Comenzi:");
    mvwprintw(win, y++, x, "b -> back");
    wrefresh(win);
    //iesim cand primim b
    while((c=wgetch(win))!='b');
    wattroff(win,COLOR_PAIR(2));
}

//functie de creare meniu cu elementele din site_cu_cuvinte
int creare_meniu(SITE *site_cu_cuvinte,int n,char text_de_cautat[])
{
    //cream iteme cu biblioteca de meniuri
    ITEM **my_items;
	int i,recursiv,x,y,xmax,ymax,xbara;
    int c,site_selectat=0;				
	MENU *my_menu;
    WINDOW *win=newwin(0,0,0,0);
    wrefresh(win);
    box(win,0,0);
    wbkgd(win,COLOR_PAIR(1));
    wattron(win,COLOR_PAIR(1));
    curs_set(0); //cursor invizibil
    x=(getmaxx(win));
    y=(getmaxy(win));
    xmax=x-20;
    ymax=y-5;
    if(!n)
    {
        //nu exista niciun site bun
        mvwprintw(win, 7, x/2-8, "No results found");
        mvwprintw(win, ymax++, xmax, "Comenzi:");
        mvwprintw(win, ymax++, xmax, "b -> back");
    }
    else
    {
        //legenda pt siteuri existente
        mvwprintw(win, ymax++, xmax, "Comenzi:");
        mvwprintw(win, ymax++, xmax, "sus/jos -> miscare");
        mvwprintw(win, ymax++, xmax, "Enter -> accesare");
        mvwprintw(win, ymax++, xmax, "b -> back");
    }
    //amplasare bara cu textul cautat
    xbara=(getmaxx(win))/2-30;
    for(i=0;i<60;i++)
    {
        mvwprintw(win, 1, xbara+i, "#");
        mvwprintw(win, 3, xbara+i, "#");
    }
    mvwprintw(win, 2, xbara, "#");
    mvwprintw(win, 2, xbara+59, "#");
    mvwprintw(win, 2, xbara+1, text_de_cautat);
    //generare meniu efectiv
    my_items=(ITEM **)calloc(n+5,sizeof(ITEM *));
    for(i=0;i<n;i++)
        my_items[i]=new_item(site_cu_cuvinte[i].url,site_cu_cuvinte[i].titlu);
    my_items[n]=(ITEM *)NULL;
    my_menu=new_menu((ITEM **)my_items);
    keypad(win, TRUE);
    set_menu_win(my_menu,win);
    set_menu_sub(my_menu,derwin(win, y-5, x-5, 5, 3));
    set_menu_mark(my_menu, "  [X]  ");
    post_menu(my_menu);
	wrefresh(win);
    //implementare taste enter b si sageti
    while((c=wgetch(win))!=10&&c!='b')
	{       switch(c)
	        {	case KEY_DOWN:
				    menu_driver(my_menu, REQ_DOWN_ITEM);
                    if(site_selectat<n-1)
                        site_selectat++;
				    break;
			    case KEY_UP:
				    menu_driver(my_menu, REQ_UP_ITEM);
                    if(site_selectat>0)
                        site_selectat--;
				    break;
		    }
            wrefresh(win);
	}
    if(c==10)
    {
        if(n!=0)
        {
            //daca exista cel putin un site, il putem accesa
            enter(site_cu_cuvinte[site_selectat]);
            //cream alt meniu cu aceleasi elemente cand iesim de pe un site
            recursiv=creare_meniu(site_cu_cuvinte,n,text_de_cautat);
        }
        else
            while((c=wgetch(win))!='b'); //asteptam b daca nu exista siteuri
    }
    if(c=='b')
        recursiv=1;
    //eliberare memorie meniu
    unpost_menu(my_menu);
    free_menu(my_menu);
    for(i=0;i<n;i++)
        free_item(my_items[i]);
    endwin();
    return recursiv;	
}

//cautarea simpla
//functie care incarca din vectorul mare in site_cu_cuvinte
//nr == de elemente din site
int parsare_site_cuvinte_simplu(SITE *site_cu_cuvinte,
                            SITE *site,int nr,char text_de_cautat[])
{
    int i,j=0;
    char *p,rezerva[101];
    strcpy(rezerva,text_de_cautat);
    for(i=0;i<nr;i++)
        site[i].valid=1;
    //salvam intr-un sir de caractere ce primim de la tastatura
    char *word=strtok(rezerva," \n");
    while(word!=NULL)
    {
        //cautam in fiecare site
        for(i=0;i<nr;i++)
        {
            p=strstr(site[i].continut,word);
            //daca am gasit cuvantul si siteul poate fi pus in site_cu_cuvinte
            if(p!=NULL&&site[i].valid==1)
            {
                //conditie pt a anula cazul cuvant in cuvant
                if((p==site[i].continut||!isalpha(p[-1]))&&
                   (p[strlen(word)]==0||!isalpha(p[strlen(word)])))
                {
                    site[i].valid=0; //siteul nu mai poate fi pus a2a oara
                    site_cu_cuvinte[j++]=site[i];
                }
            }
        }
        word=strtok(NULL," \n");
    }
    return j;
}

//functie pentru cautarea avansata
//pune in site_cu_cuvinte siteurile ce respecta conditiile
int parsare_site_cuvinte_avansat(SITE *site_cu_cuvinte,
                            SITE *site,int nr,char text_de_cautat[])
{
    int i,j=0,k;
    char rezerva[101],cuvant[101],*p;
    for(i=0;i<nr;i++)
        site[i].valid=1;
    strcpy(rezerva,text_de_cautat);
    //excludere siteuri cu cuvinte cu -
    //parcurgem toata linia
    for(k=0;k<strlen(text_de_cautat);k++)
    {
        //cand avem ghilimele sarim peste ele
        if(rezerva[k]=='\"')
        {
            k++;
            while(rezerva[k]!='\"')
                k++;
        }
        //daca gasim un "-" intai vedem daca nu este in cuvant ex: Cluj-Napoca
        if((rezerva[k]=='-')&&(k==0||rezerva[k-1]==' '))
        {
            if(rezerva[k+1]!=' '&&rezerva[k+1]!='\n')
            {
                //din pozitia actuala, scoatem cuvantul din linie(rezerva)
                //si invalidam siteurile ce il contin
                char *word=strtok(rezerva+k+1," \n");
                for(i=0;i<nr;i++)
                {
                    p=strstr(site[i].continut,word);
                    if(p!=NULL)
                    {
                        //verificam cazul cuvant in cuvant
                        if((p==site[i].continut||!isalpha(p[-1]))&&
                        (p[strlen(word)]==0||!isalpha(p[strlen(word)])))
                            site[i].valid=0;
                    }
                }
            }
        }
    }

    //verificare siteuri cu cuvinte bune
    for(k=0;k<strlen(text_de_cautat);k++)
    {
        //salvam fiecare cuvant in sir de caractere
        cuvant[0]='\0';
        int poz_cuv=0; //lungimea cuvantului/secventei
        //daca gasim " ii cautam perechea
        if(text_de_cautat[k]==34)
        {
            k++;
            while(text_de_cautat[k]!=34)
            {
                cuvant[poz_cuv++]=text_de_cautat[k++];
            }
            cuvant[poz_cuv]='\0';
            //cautare de siteuri ce contin cuvant/secventa
            for(i=0;i<nr;i++)
            {
                p=strstr(site[i].continut,cuvant);
                if(p!=NULL&&site[i].valid==1)
                {
                    //verificare cuvant in cuvant
                    if((p==site[i].continut||!isalpha(p[-1]))&&
                   (p[strlen(cuvant)]==0||!isalpha(p[strlen(cuvant)])))
                   {
                       site[i].valid=0;
                       site_cu_cuvinte[j++]=site[i];
                   }
                }
            }
        }
        else
        {
            //daca dam de spatiu si nu este intre ", asamblam un cuvant normal
            if(text_de_cautat[k]!=' '&&text_de_cautat[k]!='\n'
                                            &&text_de_cautat[k]!='\0')
            {
                while(text_de_cautat[k]!=' '&&text_de_cautat[k]!='\n'
                                                &&text_de_cautat[k]!='\0')
                {
                    cuvant[poz_cuv++]=text_de_cautat[k++];
                }
                cuvant[poz_cuv]='\0';
                //puts(cuvant);
                for(i=0;i<nr;i++)
                {
                    p=strstr(site[i].continut,cuvant);
                    if(p!=NULL&&site[i].valid==1)
                    {
                        //caz cuvant in cuvant
                        if((p==site[i].continut||!isalpha(p[-1]))&&
                       (p[strlen(cuvant)]==0||!isalpha(p[strlen(cuvant)])))
                       {
                           site[i].valid=0;
                           site_cu_cuvinte[j++]=site[i];
                       }
                    }
                }

            }
        }
    }
    return j;
}

//functie care initiaza ambele tipuri de cautare
//si care citeste din fisiere
//si care porneste meniul
int parsare_completa(int decizie,char text_de_cautat[])
{
    FILE *f, *f1,*fout;
    SITE *site;
    int i=0,j,memorie=3,recursiv=0;
    char html_de_citit[31],vect[101];
    site=(SITE *)malloc(memorie*sizeof(SITE));
    //deschidem si citim din master
    f=fopen("master.txt","r");
    if(f)
    {
        while(fgets(html_de_citit,31,f)!=NULL)
        {
            //scapam de '\n'-ul de la final
            html_de_citit[strcspn(html_de_citit,"\n")]=0;
            //deschidem si citim din fiecare site
            f1=fopen(html_de_citit,"r");
            if(f1)
            {
                //i -> la al cata-lea site suntem
                if(i%3==0&&i>0)
                {
                    memorie+=3;
                    site=(SITE *)realloc(site,memorie*sizeof(SITE));
                }
                //citim prima linie
                fscanf(f1,"%s",site[i].url);
                fscanf(f1,"%d",&site[i].lungime);
                fscanf(f1,"%d",&site[i].accesari);
                fscanf(f1,"%d",&site[i].checksum);
                fgetc(f1);
                strcpy(site[i].nume_fis,html_de_citit);
                //culori default
                site[i].fundal=white;
                site[i].text=black;
                site[i].cod=(char *)malloc((site[i].lungime+3)*sizeof(char));
                site[i].cod[0]='\0';
                while(fgets(vect,101,f1))
                {
                    strcat(site[i].cod,vect);
                }
                //obtinerea titlului
                titlul(&site[i]);
                //obtinerea continutului
                continutul(&site[i]);
                fclose(f1);
                i++;
            }
        }
        fclose(f);
    }
    SITE *site_cu_cuvinte;
    site_cu_cuvinte=(SITE *)malloc(memorie*sizeof(SITE));
    //in functie de litera primita se alege simplu sau avansat
    if(decizie)
        j=parsare_site_cuvinte_simplu(site_cu_cuvinte,site,i,text_de_cautat);
    else
        j=parsare_site_cuvinte_avansat(site_cu_cuvinte,site,i,text_de_cautat);
    //declansare meniu
    //j - nr de site_cu_cuvinte
    recursiv=creare_meniu(site_cu_cuvinte,j,text_de_cautat);
    return recursiv;
}

//functie care creeaza bara de cautare si declanseaza cautarea efectiva
void cautare()
{
    int x,y,i,j,poz=0,ok=0,increment_text=0,recursiv,xmax,ymax;
    char litera,text_de_cautat[101]="";
    text_de_cautat[0]='\0';
    //creare fereastra noua
    WINDOW *win=newwin(0,0,0,0);
    wrefresh(win);
    box(win,0,0);
    wbkgd(win,COLOR_PAIR(1));
    wattron(win,COLOR_PAIR(1));
    curs_set(2); //cursor vizibil
    //amplasare bara de cautare pe mijloc
    x=(getmaxx(win))/2-30;
    y=(getmaxy(win))/2-1;
    for(i=0;i<60;i++)
    {
        mvwprintw(win, y, x+i, "#");
        mvwprintw(win, y+2, x+i, "#");
    }
    mvwprintw(win, y+1, x, "#");
    mvwprintw(win, y+1, x+59, "#");
    //scrierea legendei
    xmax=(getmaxx(win))-20;
    ymax=(getmaxy(win))-5;
    mvwprintw(win, ymax++, xmax, "Comenzi:");
    mvwprintw(win, ymax, xmax, "Enter -> blocare");
    wrefresh(win);
    //se misca cursorul
    wmove(win,y+1,x+1);
    //asteptam sa primim enter
    while((litera=wgetch(win))!='\n')
    {
        //daca nu e enter, adaugam in text
        text_de_cautat[increment_text++]=litera;
        char sir[2];
        sir[0]=litera;
        sir[1]='\0';
        mvwprintw(win, y+1, x+1+poz, sir);
        poz++;
        wmove(win,y+1,x+1+poz);
        wrefresh(win);
    }
    text_de_cautat[increment_text]='\0';
    curs_set(0);
    //updatare legenda
    mvwprintw(win, ymax++, xmax, "s->cautare simpla");
    mvwprintw(win, ymax++, xmax, "a->cautare avansata");
    mvwprintw(win, ymax++, xmax, "q->quit");
    wrefresh(win);
    //textul este blocat, asteptam litera
    while(litera=wgetch(win))
    {
        switch (litera)
        {
            case 's':
                //cautare simpla
                recursiv=parsare_completa(1,text_de_cautat);
                ok=1;
                if(recursiv)
                    cautare();
                break;
            case 'a':
                //cautare avansata
                recursiv=parsare_completa(0,text_de_cautat);
                ok=1;
                if(recursiv)
                    cautare();
                break;
            case 'q':
                //inchidem fereastra
                endwin();
                ok=1;
                break;
            default:
                break;
        }
        //se iese doar daca primim s a sau q
        if(ok)
            break;
    }
    wrefresh(win);
}

//functie care pune numele browserului pe mijloc
void nume_motor(WINDOW *win)
{
    int x,y;
    char titlu_premium_2k22[]="Mo(god)zilla";
    x=(getmaxx(win))/2-strlen(titlu_premium_2k22)/2;
    y=(getmaxy(win))/2;
    mvwprintw(win, y, x, titlu_premium_2k22);
    //plasare legenda
    x=(getmaxx(win))-20;
    y=(getmaxy(win))-5;
    mvwprintw(win, y++, x, "Comenzi:");
    mvwprintw(win, y++, x, "c -> cautare");
    mvwprintw(win, y++, x, "q -> quit");
    wrefresh(win);
}

int main()
{
    char litera;
    int ok=0;
    //creare ecran principal
    initscr();
    start_color();
    noecho();
    init_pair(1,COLOR_BLACK, COLOR_YELLOW);
    WINDOW *win=newwin(0,0,0,0);
    wrefresh(win);
    box(win,0,0);
    wbkgd(win,COLOR_PAIR(1));
    wattron(win,COLOR_PAIR(1));
    curs_set(0); //cursor invizibil
    //scrie nume browser
    nume_motor(win);
    //asteptam litera pana vine c sau q
    while(litera=wgetch(win))
    {
        switch (litera)
        {
            case 'c':
                cautare();
                ok=1;
                break;
            case 'q':
                endwin();
                ok=1;
                break;
            default:
                break;
        }
        if(ok)
            break;
    }
    curs_set(2);
    //inchidere completa
    wattroff(win,COLOR_PAIR(1));
    endwin();
    return 0;
}