#include <bits/stdc++.h>
#include <pthread.h>

using namespace std;

pthread_t keyboard, window, add, rmv, shw, hlp;
pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

vector<string> reproduction_list;
string song_to_add;
string song_to_rmv;

void* window_manag(void* arg){
    printf("--- Welcome to TUI Music Player ---\n\n");
    printf("Usage:\n");
    printf("    Enter command: add -- to add a music to reproduction list\n \
                  rmv -- to remove a music from reproduction list\n \
                  shw -- to show the reproduction list\n \
                  hlp -- to show the usage instructions\n \
                  ext -- to exit\n\n");
    
    pthread_exit(NULL);
}

void* add_music(void* arg){
    while(pthread_mutex_trylock(&_mutex));

    if(song_to_add.length() == 1 || song_to_add.empty())
        cout << "ERROR: You must type a song!\n" << endl;
    
    else{
        reproduction_list.push_back(song_to_add);
        cout << "The music was sucessfully added to the list!\n" << endl;
    }

    pthread_mutex_unlock(&_mutex);
    pthread_exit(NULL);
}

void* rmv_music(void* arg){
    for (int i = 0; i < reproduction_list.size(); i++){
        if(reproduction_list[i] == song_to_rmv){ 

            while(pthread_mutex_trylock(&_mutex));

            reproduction_list.erase(reproduction_list.begin()+i);
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

    if (reproduction_list.empty())
        cout << "Sorry, the list is empty!\n" << endl;
    
    else{
        cout << "\n--- Playlist ---" << endl;
        for(auto elem : reproduction_list)
            cout << "   " << elem << endl;
        cout << endl;
    }    

    pthread_mutex_unlock(&_mutex);
    pthread_exit(NULL);
}

void* send_help(void* arg){
    printf("Usage:\n");
    printf("    Enter command: add -- to add a music to reproduction list\n \
                  rmv -- to remove a music from reproduction list\n \
                  shw -- to show the reproduction list\n \
                  hlp -- to show the usage instructions\n \
                  ext -- to exit\n\n");
    
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

            pthread_create(&rmv, NULL, &rmv_music, NULL);
            pthread_join(rmv, NULL);            
        }

        else if (cmd == "shw"){
            pthread_create(&shw, NULL, &shw_queue, NULL);
            pthread_join(shw, NULL);
        }

        else if (cmd == "hlp"){
            pthread_create(&hlp, NULL, &send_help, NULL);
            pthread_join(hlp, NULL);
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