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
    printf("    Enter command: add -- to add a music to reprotudion queue\n \
                  rmv -- to remove a music from reprodution queue\n \
                  ext -- to exit\n\n");
    
    pthread_exit(NULL);
}

void* add_queue(void* music_name){
    while(pthread_mutex_trylock(&_mutex));

    reprodution_queue.push_back(music_name);
    cout << "The music was sucessfully added to the list!\n" << endl;

    pthread_mutex_unlock(&_mutex);
    pthread_exit(NULL);
}

void* rmv_queue(void* music_name){

    for (int i = 0; i < reprodution_queue.size(); i++){
        string item = *reinterpret_cast<string*>(reprodution_queue[i]);
        string music = *reinterpret_cast<string*>(music_name);

        cout << item << endl; // BUG BUG BUG BUG --> rmv opa  
        cout << music << endl;

        if(item == music){ 

            while(pthread_mutex_trylock(&_mutex));

            reprodution_queue.erase(reprodution_queue.begin()+i);
            cout << "The music was sucessfully deleted!\n" << endl;;

            pthread_mutex_unlock(&_mutex);
            pthread_exit(NULL);
        }
    }
    cout << "ERROR: Music not found!\n" << endl;
    pthread_exit(NULL);
}

void* keyboard_manag(void* arg){
    string cmd;

    while(true){
        cin >> cmd;

        if (cmd == "add"){
            string music; 
            getline(cin, music); 

            pthread_create(&add, NULL, &add_queue, &music);
            pthread_join(add, NULL);
        }

        else if (cmd == "rmv"){
            string music; 
            getline(cin, music);
            
            pthread_create(&add, NULL, &rmv_queue, &music);
            pthread_join(rmv, NULL);            
        }

        else if(cmd == "ext")
            pthread_exit(NULL);

        else{
            cout << "ERROR: Command not found!\n" << endl; 
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