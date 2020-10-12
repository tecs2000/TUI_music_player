#include <bits/stdc++.h>
#include <pthread.h>

using namespace std;

pthread_t keyboard, window, add, rmv, shw;
pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

vector<string> reprodution_music;
string song_to_add;
string song_to_rmv;

void* window_manag(void* arg){
    printf("--- Welcome to TUI Music Player ---\n\n");
    printf("Usage:\n");
    printf("    Enter command: add -- to add a music to reproduction list\n \
                  rmv -- to remove a music from reproduction list\n \
                  ext -- to exit\n \
                  shw -- to show the reproduction list\n\n");
    
    pthread_exit(NULL);
}

void* add_music(void* arg){
    while(pthread_mutex_trylock(&_mutex));

    if(song_to_add.length() == 1 || song_to_add.empty())
        cout << "ERROR: You must type a song!\n" << endl;
    
    else{
        reprodution_music.push_back(song_to_add);
        cout << "The music was sucessfully added to the list!\n" << endl;
    }

    pthread_mutex_unlock(&_mutex);
    pthread_exit(NULL);
}

void* rmv_music(void* arg){
    for (int i = 0; i < reprodution_music.size(); i++){
        if(reprodution_music[i] == song_to_rmv){ 

            while(pthread_mutex_trylock(&_mutex));

            reprodution_music.erase(reprodution_music.begin()+i);
            cout << "The music was sucessfully deleted!\n" << endl;;

            pthread_mutex_unlock(&_mutex);
            pthread_exit(NULL);
        }
    }
    cout << "ERROR: Music not found!\n" << endl;
    pthread_exit(NULL);
}

void* shw_queue(void* arg){
    while(pthread_mutex_trylock(&_mutex));

    cout << "\n--- Playlist ---" << endl;
    for(auto elem : reprodution_music)
        cout << "   " << elem << endl;
    cout << endl;

    pthread_mutex_unlock(&_mutex);
    pthread_exit(NULL);
}

void* keyboard_manag(void* arg){
    string cmd;

    while(true){
        cin >> cmd;

        if (cmd == "add"){
            string music_name; 
            getline(cin, music_name); 

            song_to_add = music_name; 

            pthread_create(&add, NULL, &add_music, NULL);
            pthread_join(add, NULL);
        }

        else if (cmd == "rmv"){
            string music_name; 
            getline(cin, music_name);
            
            song_to_rmv = music_name;

            pthread_create(&add, NULL, &rmv_music, NULL);
            pthread_join(rmv, NULL);            
        }

        else if (cmd == "shw"){
            pthread_create(&shw, NULL, &shw_queue, NULL);
            pthread_join(shw, NULL);
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