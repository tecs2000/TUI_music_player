#include <bits/stdc++.h>
// #include <ncurses.h>
#include <pthread.h>

using namespace std;

pthread_t keyboard, window, add, rmv;
pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;
vector<void*> reprodution_queue;

// void *window_manag(void* arg){
//     initscr();
//     noecho();

//     printw("--- Welcome to TUI Music Player ---\n\n");
//     printw("Usage:\n");
//     printw("    Press command: a -- to add a music to reprotudion queue\n \
//                   r -- to remove a music from reprodution queue\n \
//                   d -- to exit\n");

//     while(true){
//         char key = getch();
//         if(key == 'd'){
//             endwin();
//             pthread_exit(NULL);
//         }       
//     }
// }

void* window_manag(void* arg){
    printf("--- Welcome to TUI Music Player ---\n\n");
    printf("Usage:\n");
    printf("    Enter command: a -- to add a music to reprotudion queue\n \
                  r -- to remove a music from reprodution queue\n \
                  d -- to exit\n");
    
    pthread_exit(NULL);
}

void* add_queue(void* music_name){
    while(pthread_mutex_trylock(&_mutex));

    reprodution_queue.push_back(music_name);
    printf("The music was sucessfully added to the list!");

    pthread_mutex_unlock(&_mutex);

    pthread_exit(NULL);
}

void* keyboard_manag(void* arg){
    string cmd;
    while(true){
        cin >> cmd;
        if (cmd == "a"){
            char* music; 
            cin >> music; // ----> CRASH
            printf("%d", *music); // NAO CHEGA AQUI
            pthread_create(&add, NULL, &add_queue, (void*) music);
        }
    
        else if(cmd == "d"){
            pthread_exit(NULL);
            break;
        }
    }
}

int main(){

    pthread_create(&window, NULL, window_manag, NULL);
    pthread_create(&keyboard, NULL, keyboard_manag, NULL);

    pthread_join(window, NULL);
    pthread_join(keyboard, NULL);

    pthread_exit(NULL);
}