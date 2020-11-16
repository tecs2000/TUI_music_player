#include <bits/stdc++.h>
#include <pthread.h>
#include <ncurses.h>
#include <unistd.h>

using namespace std;

// -- GLOBAIS -- 
WINDOW *Comandos, *Mostrador, *Biblioteca; 

pthread_t threads[2];

pthread_mutex_t BARRA_PROGRESSO_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t BIBLIOTECA_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MODO_REPRODUCAO_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PLAYING_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t TIMER_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t INDEX_MUSICA_ATUAL_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t DURACAO_MUSICA_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t FILA_REPRODUCAO_MUTEX = PTHREAD_MUTEX_INITIALIZER;

typedef struct musica{
    char autor[20];
    char nome[20];
    int duracao;
}musica;

vector<musica> fila_de_reproducao;
set<int> musicas_tocadas;

bool PLAYING = false;
int index_musica_atual = -1, timer_atual = -1, duracao_musica_atual = 0;

string MODO_REPRODUCAO = "SEQUENCIAL";

// -- FUNCOES UTILITARIAS -- 
void set_modo_reproducao(){

    while(pthread_mutex_trylock(&MODO_REPRODUCAO_MUTEX));

    if(MODO_REPRODUCAO == "ALEATORIO") MODO_REPRODUCAO = "SEQUENCIAL";
    else MODO_REPRODUCAO = "ALEATORIO";

    pthread_mutex_unlock(&MODO_REPRODUCAO_MUTEX);
}


void set_play_pause(){

    while(pthread_mutex_trylock(&PLAYING_MUTEX));

    PLAYING = !PLAYING;

    pthread_mutex_unlock(&PLAYING_MUTEX);
}


void exibir_barra_de_progresso(){

    while(pthread_mutex_trylock(&TIMER_MUTEX));
    while(pthread_mutex_trylock(&BARRA_PROGRESSO_MUTEX));
    while(pthread_mutex_trylock(&DURACAO_MUSICA_MUTEX));

    wmove(Mostrador, 1, 0.50*COLS+1);
    wprintw(Mostrador, "%0.2d:%0.2d ", timer_atual/60, timer_atual%60);

    for(int i = 0.50*COLS + 7; i < COLS-8; i++) // esse for printa a linha de progresso
        waddch(Mostrador, ACS_HLINE);

    wprintw(Mostrador, " %0.2d:%0.2d", duracao_musica_atual/60, duracao_musica_atual%60);
    
    wrefresh(Mostrador);

    pthread_mutex_unlock(&DURACAO_MUSICA_MUTEX);
    pthread_mutex_unlock(&BARRA_PROGRESSO_MUTEX);
    pthread_mutex_unlock(&TIMER_MUTEX);
}


void exibir_modo_reproducao(){

    while(pthread_mutex_trylock(&MODO_REPRODUCAO_MUTEX));

    // exibe o MODO_REPRODUCAO de reproducao
    wattron(Mostrador, COLOR_PAIR(1));
    if(MODO_REPRODUCAO == "SEQUENCIAL")
        mvwprintw(Mostrador, 1, (COLS + 4)/4 + (COLS + 4)/8 - 8, "MODO SEQUENCIAL"); 

    else
        mvwprintw(Mostrador, 1, (COLS + 4)/4 + (COLS + 4)/8 - 8, "MODO  ALEATORIO"); 
    wattroff(Mostrador, COLOR_PAIR(1));

    wrefresh(Mostrador);
    pthread_mutex_unlock(&MODO_REPRODUCAO_MUTEX);
}


void exibir_fila_de_reproducao(){ 

    while(pthread_mutex_trylock(&FILA_REPRODUCAO_MUTEX));
    while(pthread_mutex_trylock(&BIBLIOTECA_MUTEX));

    wclear(Biblioteca);

    wborder(Biblioteca, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);
    mvwprintw(Biblioteca, 0, 3, "Musicas");
    mvwprintw(Biblioteca, 0 , (COLS/2)-3, "Artistas");
    mvwprintw(Biblioteca, 0, COLS-10, "Duracao");

    wrefresh(Biblioteca);

    if(fila_de_reproducao.empty())
        mvwprintw(Biblioteca, 0.75 * LINES / 2, COLS/2 - 12, "A biblioteca esta vazia!");

    else{
        int line = 1;   // comeca da linha 1 para nao mexer na borda

        for(auto musica : fila_de_reproducao){
            mvwprintw(Biblioteca, line, 1, "%d  %s", line, musica.nome);
            mvwaddstr(Biblioteca, line, (COLS/2)-2, musica.autor);

            int minutos = musica.duracao / 60;
            int segundos = musica.duracao % 60;
            mvwprintw(Biblioteca, line, COLS-9, "%02d:%02d", minutos, segundos); 

            while(pthread_mutex_trylock(&INDEX_MUSICA_ATUAL_MUTEX));

            if(index_musica_atual + 1 == line)
                mvwchgat(Biblioteca, line, 1, COLS-2, COLOR_PAIR(1), 1, NULL);

            pthread_mutex_unlock(&INDEX_MUSICA_ATUAL_MUTEX);

            ++line;   
        }
    }

    wrefresh(Biblioteca);
    pthread_mutex_unlock(&BIBLIOTECA_MUTEX);
    pthread_mutex_unlock(&FILA_REPRODUCAO_MUTEX);
}


void inicializar_barra_progresso(){

    while(pthread_mutex_trylock(&BARRA_PROGRESSO_MUTEX));

    wclear(Mostrador);
    
    // desenha a barra de volume
    mvwaddstr(Mostrador, 1, 1, "vol "); 
    for(int i = 0; i < 0.25*COLS - 10; i++)
        waddch(Mostrador, ACS_HLINE); 
    waddstr(Mostrador, " 50%");

    exibir_modo_reproducao();   

    // desenha a barra de tempo de execução
    mvwprintw(Mostrador, 1, 0.50*COLS+1, "00:00 ");

    for(int i = 0.50*COLS + 7; i < COLS-8; i++)
        waddch(Mostrador, ACS_HLINE);

    wprintw(Mostrador, " --:--");

    wrefresh(Mostrador);
    pthread_mutex_unlock(&BARRA_PROGRESSO_MUTEX);
}

// seta as variaveis para tocar a primeira musica
void iniciar_playlist(int inicio = 0){

    while(pthread_mutex_trylock(&TIMER_MUTEX));
    while(pthread_mutex_trylock(&INDEX_MUSICA_ATUAL_MUTEX));
    while(pthread_mutex_trylock(&DURACAO_MUSICA_MUTEX));
    while(pthread_mutex_trylock(&FILA_REPRODUCAO_MUTEX));

    timer_atual = 0;
    index_musica_atual = inicio;

    if(!fila_de_reproducao.empty())
        duracao_musica_atual = fila_de_reproducao[index_musica_atual].duracao;
    else
        duracao_musica_atual = 0;
    
    pthread_mutex_unlock(&FILA_REPRODUCAO_MUTEX);
    pthread_mutex_unlock(&DURACAO_MUSICA_MUTEX);
    pthread_mutex_unlock(&INDEX_MUSICA_ATUAL_MUTEX);
    pthread_mutex_unlock(&TIMER_MUTEX);
}


// seta as variaveis para esperarem o proximo play
void finalizar_playlist(){

    while(pthread_mutex_trylock(&TIMER_MUTEX));
    while(pthread_mutex_trylock(&INDEX_MUSICA_ATUAL_MUTEX));
    while(pthread_mutex_trylock(&DURACAO_MUSICA_MUTEX));

    timer_atual = -1;
    index_musica_atual = -1;
    duracao_musica_atual = 0;

    musicas_tocadas.clear();
    
    pthread_mutex_unlock(&DURACAO_MUSICA_MUTEX);
    pthread_mutex_unlock(&INDEX_MUSICA_ATUAL_MUTEX);
    pthread_mutex_unlock(&TIMER_MUTEX);
}

// se possivel, passa para a proxima musica e atualiza as variaveis
bool next_musica_sequencial(bool musica_atual_apagada = false){

    while(pthread_mutex_trylock(&TIMER_MUTEX));
    while(pthread_mutex_trylock(&INDEX_MUSICA_ATUAL_MUTEX));
    while(pthread_mutex_trylock(&DURACAO_MUSICA_MUTEX));
    while(pthread_mutex_trylock(&FILA_REPRODUCAO_MUTEX));

    bool sucesso = false;
    int temp = index_musica_atual + !musica_atual_apagada; //se a musica atual nao foi apagada, vai para a proxima musica
      
    //checa se 1- tem algo na fila, 2- a execucao foi iniciada, 3- se ha alguma musica depois da musica atual
    if(!fila_de_reproducao.empty() && index_musica_atual != -1 
       && (fila_de_reproducao.size() > temp)){
        
        index_musica_atual = temp;

        musicas_tocadas.insert(index_musica_atual);
        timer_atual = 0;           
        duracao_musica_atual = fila_de_reproducao[index_musica_atual].duracao;

        sucesso = true;            
    }    

    pthread_mutex_unlock(&FILA_REPRODUCAO_MUTEX);
    pthread_mutex_unlock(&DURACAO_MUSICA_MUTEX);
    pthread_mutex_unlock(&INDEX_MUSICA_ATUAL_MUTEX);
    pthread_mutex_unlock(&TIMER_MUTEX);

    if (sucesso){
        exibir_fila_de_reproducao();
        return true;
    } 
        
    else
        return false;
}


bool next_musica_aleatoria(bool musica_atual_removida = false){

    while(pthread_mutex_trylock(&TIMER_MUTEX));
    while(pthread_mutex_trylock(&INDEX_MUSICA_ATUAL_MUTEX));
    while(pthread_mutex_trylock(&DURACAO_MUSICA_MUTEX));
    while(pthread_mutex_trylock(&FILA_REPRODUCAO_MUTEX));

    bool sucesso = false;
    
    // o index da musica indica se a playlist ja foi inicializada
    // a comparacao abaixo certifica que a playlist nao ficara tocando indefinidamente
    if(!fila_de_reproducao.empty() && index_musica_atual != -1  \
        && fila_de_reproducao.size() > musicas_tocadas.size()){
        
        int proxima_musica_index;
        while(1){ // procura por uma musica ainda nao tocada 
            proxima_musica_index = rand() % fila_de_reproducao.size();

            set<int>::iterator it;
            it = musicas_tocadas.find(proxima_musica_index);

            if(it == musicas_tocadas.end()) 
                break;
        }
        
        index_musica_atual = proxima_musica_index;
        timer_atual = 0;           
        duracao_musica_atual = fila_de_reproducao[proxima_musica_index].duracao;

        musicas_tocadas.insert(proxima_musica_index);

        sucesso = true;
    }   

    pthread_mutex_unlock(&FILA_REPRODUCAO_MUTEX);
    pthread_mutex_unlock(&DURACAO_MUSICA_MUTEX);
    pthread_mutex_unlock(&INDEX_MUSICA_ATUAL_MUTEX);
    pthread_mutex_unlock(&TIMER_MUTEX);

    if (sucesso){
        exibir_fila_de_reproducao();
        return true;
    } 
        
    else
        return false;
}


void remover_musica(int musica_index){

    while(pthread_mutex_trylock(&FILA_REPRODUCAO_MUTEX));

    fila_de_reproducao.erase(fila_de_reproducao.begin() + (musica_index - 1));

    pthread_mutex_unlock(&FILA_REPRODUCAO_MUTEX);

    while(pthread_mutex_trylock(&INDEX_MUSICA_ATUAL_MUTEX)); // trava pra olhar o valor

    if(index_musica_atual == musica_index - 1){ // checa se a musica desejada esta tocando

        musicas_tocadas.erase(index_musica_atual); 

        pthread_mutex_unlock(&INDEX_MUSICA_ATUAL_MUTEX); // destrava pra dar acesso as funcoes seguintes

        if(musicas_tocadas.size() == fila_de_reproducao.size()){ // checa se ha proxima musica
            finalizar_playlist();
            inicializar_barra_progresso();
        }
        
        else{
            if(MODO_REPRODUCAO == "ALEATORIO") next_musica_aleatoria(true);
            else next_musica_sequencial(true);
        }
    }

    pthread_mutex_unlock(&INDEX_MUSICA_ATUAL_MUTEX); 
    exibir_fila_de_reproducao();
}


void inicializador_de_janelas(){
    while(pthread_mutex_trylock(&BIBLIOTECA_MUTEX));

    wborder(Biblioteca, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);
    mvwprintw(Biblioteca, 0, 3, "Musicas");
    mvwprintw(Biblioteca, 0 , (COLS/2)-3, "Artistas");
    mvwprintw(Biblioteca, 0, COLS-10, "Duracao");
    mvwprintw(Biblioteca, 0.75 * LINES / 2, COLS/2 - 12, "A biblioteca esta vazia!");

    wrefresh(Biblioteca);

    pthread_mutex_unlock(&BIBLIOTECA_MUTEX);

    inicializar_barra_progresso();  // desenha o volume e barra de progresso
}


// -- FUNCOES DE CONTROLE -- 
void* controlador_barra_progresso(void* arg){
    while(true){

        if(PLAYING && timer_atual < duracao_musica_atual){ // a musica atual nao acabou

            if(timer_atual == -1 && MODO_REPRODUCAO == "SEQUENCIAL"){   
                iniciar_playlist(); // inicia a playlist da musica 0
                musicas_tocadas.insert(0);
            }
            
            if(timer_atual == -1 && MODO_REPRODUCAO == "ALEATORIO"){
                int primeira_musica = rand() % fila_de_reproducao.size(); // inicia de forma aleatoria
                iniciar_playlist(primeira_musica);
                musicas_tocadas.insert(primeira_musica);
            }

            exibir_fila_de_reproducao();
            exibir_barra_de_progresso();
            sleep(1);

            ++timer_atual;
        }

        else if(PLAYING && timer_atual == duracao_musica_atual){

            if((MODO_REPRODUCAO == "SEQUENCIAL" && !next_musica_sequencial()) ||
               (MODO_REPRODUCAO == "ALEATORIO" && !next_musica_aleatoria())){ // a playlist chegou ao fim
                set_play_pause();   // pausa a execucao
                finalizar_playlist();
                exibir_fila_de_reproducao();    // exibe que nada esta sendo tocado
            }
            
            inicializar_barra_progresso(); // reseta a barra de progresso para a proxima musica
            sleep(1);
        }
        else
            sleep(1);   // caso a execucao esteja pausada, dorme  
    }
}


void* controlador_comandos(void* arg){
    while(true){
        noecho();

        wattron(Comandos, COLOR_PAIR(1) | A_BOLD);
        mvwaddstr(Comandos, 0, (COLS / 7) - 5, " A ");
        wattroff(Comandos, COLOR_PAIR(1) | A_BOLD);
        waddstr(Comandos, " add");

        wattron(Comandos, COLOR_PAIR(1) | A_BOLD);
        mvwaddstr(Comandos, 0, 2 * (COLS / 7) - 7, " P ");
        wattroff(Comandos, COLOR_PAIR(1) | A_BOLD);
        waddstr(Comandos, " play/pause");

        wattron(Comandos, COLOR_PAIR(1) | A_BOLD);
        mvwaddstr(Comandos, 0, 3 * (COLS / 7) - 3, " N ");
        wattroff(Comandos, COLOR_PAIR(1) | A_BOLD);
        waddstr(Comandos, " next");
        
        wattron(Comandos, COLOR_PAIR(1) | A_BOLD);
        mvwaddstr(Comandos, 0, 4 * (COLS / 7) - 5, " R ");
        wattroff(Comandos, COLOR_PAIR(1) | A_BOLD);
        waddstr(Comandos, " remove");

        wattron(Comandos, COLOR_PAIR(1) | A_BOLD);
        mvwaddstr(Comandos, 0, 5 * (COLS / 7) - 5, " C ");
        wattroff(Comandos, COLOR_PAIR(1) | A_BOLD);
        waddstr(Comandos, " change mode");

        wattron(Comandos, COLOR_PAIR(1) | A_BOLD);
        mvwaddstr(Comandos, 0, 6 * (COLS / 7) + 1, " Q ");
        wattroff(Comandos, COLOR_PAIR(1) | A_BOLD);
        waddstr(Comandos, " quit");

        wrefresh(Comandos);

        char cmd = wgetch(Comandos);

        if(cmd == 'q' || cmd == 'Q'){
            endwin();
            exit(0);
        }

        else if(cmd == 'A' || cmd == 'a'){
            musica new_song;

            echo(); 
            wclear(Comandos);

            mvwprintw(Comandos, 0, 1, "Digite o nome da musica: ");     
            wgetnstr(Comandos, new_song.nome, 20);
            wrefresh(Comandos);
            wclear(Comandos);

            mvwprintw(Comandos, 0, 1, "Digite o nome do artista: ");    
            wgetnstr(Comandos, new_song.autor, 20);
            wrefresh(Comandos);
            wclear(Comandos);

            int min = 0, seg = 0;
            mvwprintw(Comandos, 0, 1, "Digite a duracao da musica (min seg): ");    
            wscanw(Comandos, "%d %d", &min, &seg); 
            wrefresh(Comandos);                  
            wclear(Comandos);

            new_song.duracao = 60*min + seg;

            if(new_song.duracao == 0){
                wclear(Comandos);

                wattron(Comandos, COLOR_PAIR(2));
                mvwprintw(Comandos, 0, COLS/2 - 21, "A musica deve ter duracao maior que zero!"); 
                wattroff(Comandos, COLOR_PAIR(2));

                wrefresh(Comandos);
                sleep(2);
            }

            else{
                while(pthread_mutex_trylock(&FILA_REPRODUCAO_MUTEX));

                fila_de_reproducao.push_back(new_song);
                
                pthread_mutex_unlock(&FILA_REPRODUCAO_MUTEX);
                
                exibir_fila_de_reproducao();
            }

            wclear(Comandos);
        }

        else if(cmd == 'p' || cmd == 'P'){
            if(fila_de_reproducao.empty()){
                wclear(Comandos);

                wattron(Comandos, COLOR_PAIR(2));
                mvwprintw(Comandos, 0, COLS/2 - 12, "A biblioteca esta vazia!"); 
                wattroff(Comandos, COLOR_PAIR(2));

                wrefresh(Comandos);
                sleep(2);
                wclear(Comandos);
            }

            else
                set_play_pause();
        }

        else if(cmd == 'n' || cmd == 'N'){
            // checa se ha proxima musica, se nao houver, printa msg de erro
            if((MODO_REPRODUCAO == "SEQUENCIAL" && !next_musica_sequencial()) \
               || (MODO_REPRODUCAO == "ALEATORIO" && !next_musica_aleatoria())){
                
                wclear(Comandos);

                wattron(Comandos, COLOR_PAIR(2));
                mvwaddstr(Comandos, 0, (COLS/2) - 12, "Nao ha uma proxima musica!");
                wrefresh(Comandos);
                wattroff(Comandos, COLOR_PAIR(2));
                
                wrefresh(Comandos);
                sleep(2);
                wclear(Comandos);
            }
        }

        else if(cmd == 'c' || cmd == 'C'){
            set_modo_reproducao();
            exibir_modo_reproducao();
        }

        else if(cmd == 'r' || cmd == 'R'){
            echo(); 
            wclear(Comandos);
            
            int indice_musica;
            mvwprintw(Comandos, 0, 1, "Digite o indice da musica a ser removida: ");     
            wscanw(Comandos, "%d", &indice_musica); 
            wrefresh(Comandos);

            // checa se o index desejado eh valido, se nao, printa msg de erro
            if(indice_musica > fila_de_reproducao.size() || indice_musica <= 0){ 
                wclear(Comandos);

                wattron(Comandos, COLOR_PAIR(2));
                mvwaddstr(Comandos, 0, (COLS/2) - 12, "Indice invalido!");
                wrefresh(Comandos);
                wattroff(Comandos, COLOR_PAIR(2));
                sleep(2);
            }
            else
                remover_musica(indice_musica);
            
            wclear(Comandos);
        }
    }
}  


int main(){
    initscr();

    start_color();
    init_pair(1, COLOR_BLACK, COLOR_YELLOW);
    init_pair(2, COLOR_RED, COLOR_YELLOW);

    Mostrador = newwin(0.125 * LINES, COLS, 0.75 * LINES, 0);
    Comandos = newwin(0.125 * LINES - 1, COLS, 0.875 * LINES + 1, 0);
    Biblioteca = newwin(0.75 * LINES, COLS, 0, 0);

    inicializador_de_janelas();    

    pthread_create(&threads[0], NULL, controlador_comandos, NULL);
    pthread_create(&threads[1], NULL, controlador_barra_progresso, NULL);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    endwin();
}





