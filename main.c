#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 100

// Şarkı yapısı (ÇİFT YÖNLÜ BAĞLI LİSTE KULLANILDI)
typedef struct Song {
    char name[100];
    char artist[100];
    struct Song* next;
    struct Song* prev;
} Song;

// Playlist yapısı (BİNARY SEARCH TREE KULLANILDI)
typedef struct PlaylistNode {
    char name[100];
    Song* head;
    struct PlaylistNode* left;
    struct PlaylistNode* right;
} PlaylistNode;

// Rating için yapı
typedef struct SongRating {
    char name[100];
    char artist[100];
    int rating;
    struct SongRating* next;
} SongRating;

// Global değişkenler
PlaylistNode* playlistRoot = NULL;
PlaylistNode* allSongsPlaylist = NULL;
PlaylistNode* currentPlaylist = NULL;
SongRating* ratingHashTable[HASH_SIZE] = {NULL};

// Hash fonksiyonu
unsigned int hash(char* str) {
    unsigned int hash = 0;
    for(int i = 0; str[i] != '\0'; i++) {
        hash = 31 * hash + str[i];
    }
    return hash % HASH_SIZE;
}

// Rating ekleme
void addRating(char* artist, char* name, int rating) {
    unsigned int index = hash(name);
    
    // aynı şarkı için var olan ratingi bul
    SongRating* current = ratingHashTable[index];
    while (current != NULL) {
        // Aynı şarkı için birden fazla rating varsa günceller
        if (strcmp(current->name, name) == 0 && strcmp(current->artist, artist) == 0) {
            current->rating = rating;
            printf("%s - %s şarkısının puanı %d olarak güncellendi.\n", artist, name, rating);
            return;
        }
        current = current->next;
    }
    
    // Eğer daha önce rating yoksa yeni rating ekle
    SongRating* newRating = (SongRating*)malloc(sizeof(SongRating));
    strcpy(newRating->name, name);
    strcpy(newRating->artist, artist);
    newRating->rating = rating;
    newRating->next = ratingHashTable[index];
    ratingHashTable[index] = newRating;
    
    printf("%s - %s şarkısına %d puan verildi.\n", artist, name, rating);
}

// Rating listesi
void listRatedSongs() {
    SongRating* allRatings[HASH_SIZE * 10];
    int ratingCount = 0;
    
    for(int i = 0; i < HASH_SIZE; i++) {
        SongRating* current = ratingHashTable[i];
        while(current != NULL) {
            allRatings[ratingCount++] = current;
            current = current->next;
        }
    }
    //bubble sort büyükten küçüğe sıralama yapıldı
    for(int i = 0; i < ratingCount - 1; i++) {
        for(int j = 0; j < ratingCount - i - 1; j++) {
            if(allRatings[j]->rating < allRatings[j+1]->rating) {
                SongRating* temp = allRatings[j];
                allRatings[j] = allRatings[j+1];
                allRatings[j+1] = temp;
            }
        }
    }
    
    printf("Şarkılar Puanlarına Göre Sıralandı:\n");
    for(int i = 0; i < ratingCount; i++) {
        printf("%s - %s: %d Puan\n",
               allRatings[i]->artist,
               allRatings[i]->name,
               allRatings[i]->rating);
    }
}

//anlık çalan şarkıyı silen fonksiyon
void deleteCurrentSong(PlaylistNode* playlist) {
    if (playlist->head == NULL) {
        printf("Oynatma listesi boş. Silinecek şarkı yok.\n");
        return;
    }

    Song* current = playlist->head;

    if (current->next == current) { // Tek bir şarkı varsa
        free(current);
        playlist->head = NULL;
    } else {
        Song* prev = current->prev;
        Song* next = current->next;
        prev->next = next;
        next->prev = prev;
        playlist->head = next; // Sonraki şarkıyı baş yap
        free(current);
    }

    printf("Şarkı silindi.\n");
}

//şarkı yaratan fonksiyon
Song* createSong(char* name, char* artist) {
    Song* newSong = (Song*)malloc(sizeof(Song));
    strcpy(newSong->name, name);
    strcpy(newSong->artist, artist);
    newSong->next = NULL;
    newSong->prev = NULL;
    return newSong;
}

//playliste şarkı ekleyen fonksiyon
void addSongToPlaylist(PlaylistNode* playlist, char* artist, char* name) {
    Song* newSong = createSong(name, artist);
    if (playlist->head == NULL) {
        playlist->head = newSong;
        newSong->next = newSong;
        newSong->prev = newSong;
    } else {
        Song* tail = playlist->head->prev;
        tail->next = newSong;
        newSong->prev = tail;
        newSong->next = playlist->head;
        playlist->head->prev = newSong;
    }

    printf("%s - %s %s oynatma listesine eklendi.\n", artist, name, playlist->name);
}

//bst ile alfabetik dizilen bir playlistler ağacı
PlaylistNode* insertPlaylist(PlaylistNode* node, char* name) {
    if (node == NULL) {
        PlaylistNode* newNode = (PlaylistNode*)malloc(sizeof(PlaylistNode));
        strcpy(newNode->name, name);
        newNode->head = NULL;
        newNode->left = newNode->right = NULL;
        return newNode;
    }

    int compareResult = strcmp(name, node->name);
    if (compareResult < 0)
        node->left = insertPlaylist(node->left, name);
    else if (compareResult > 0)
        node->right = insertPlaylist(node->right, name);
    else
        return node;

    return node;
}

//alfabetik diizlen ağacı inorder ile dolaşıp listeleyen fonksiyonlar
void listPlaylistsInorder(PlaylistNode* node, int* counter) {
    if (node == NULL) return;

    listPlaylistsInorder(node->left, counter);
    printf("%d- %s\n", (*counter)++, node->name);
    listPlaylistsInorder(node->right, counter);
}

void listPlaylists() {
    if (playlistRoot == NULL) {
        printf("Henüz hiçbir oynatma listesi oluşturulmamış.\n");
        return;
    }
    printf("Oynatma Listeleri:\n");
    int counter = 1;
    listPlaylistsInorder(playlistRoot, &counter);
}


PlaylistNode* searchPlaylistByNumber(PlaylistNode* node, int* current, int target) {
    if (node == NULL) return NULL;

    PlaylistNode* leftResult = searchPlaylistByNumber(node->left, current, target);
    if (leftResult != NULL) return leftResult;

    if (*current == target) return node;
    (*current)++;

    return searchPlaylistByNumber(node->right, current, target);
}

PlaylistNode* selectPlaylistExcludingAllSongs() {
    if (playlistRoot == NULL) {
        printf("Henüz hiçbir oynatma listesi oluşturulmamış.\n");
        return NULL;
    }

    listPlaylists();
    printf("Bir oynatma listesi numarası giriniz: ");
    int choice;
    scanf("%d", &choice);

    int current = 1;
    PlaylistNode* selectedPlaylist = searchPlaylistByNumber(playlistRoot, &current, choice);

    if (selectedPlaylist == NULL) {
        printf("Oynatma listesi bulunamadı.\n");
    }

    return selectedPlaylist;
}

PlaylistNode* selectPlaylist() {
    if (playlistRoot == NULL) {
        printf("Henüz hiçbir oynatma listesi oluşturulmamış.\n");
        return NULL;
    }

    listPlaylists();
    printf("Bir oynatma listesi numarası giriniz: ");
    int choice;
    scanf("%d", &choice);

    int current = 1;
    PlaylistNode* selectedPlaylist = searchPlaylistByNumber(playlistRoot, &current, choice);

    if (selectedPlaylist == NULL) {
        printf("Oynatma listesi bulunamadı.\n");
    }

    return selectedPlaylist;
}

//playlist oluşturan fonksiyon
void addPlaylist(char* name) {
    playlistRoot = insertPlaylist(playlistRoot, name);
    printf("Yeni oynatma listesi oluşturuldu: %s\n", name);
}

//şarkı arama
void searchSong(PlaylistNode* playlist) {
    char searchTerm[100];
    printf("Aramak istediğiniz şarkı veya sanatçı adını girin: ");
    scanf("%s", searchTerm);

    Song* temp = playlist->head;
    int found = 0;

    if (temp == NULL) {
        printf("Oynatma listesi boş.\n");
        return;
    }

    do {
        if (strstr(temp->name, searchTerm) != NULL || strstr(temp->artist, searchTerm) != NULL) {
            printf("Bulunan Şarkı: %s - %s\n", temp->artist, temp->name);
            found = 1;
        }
        temp = temp->next;
    } while (temp != playlist->head);

    if (!found) {
        printf("Arama sonucu bulunamadı.\n");
    }
}

void showMenu() {
    
    if (currentPlaylist == NULL) {
        currentPlaylist = allSongsPlaylist;
    }

    while (1) {
        printf("\n+------------------------------+\n");
        printf("|        Müzik Oynatıcı        |\n");
        printf("+------------------------------+\n");
        

        printf("| Aktif Playlist: %s\n", currentPlaylist->name);
        printf("|                              |\n");
        
        if (currentPlaylist->head) {
            printf("| Şarkı: %s\n", currentPlaylist->head->name);
            printf("| Sanatçı: %s\n", currentPlaylist->head->artist);
        } else {
            printf("| Şarkı: [Yok]                 |\n");
            printf("| Sanatçı: [Yok]               |\n");
        }
        
        printf("|                              |\n");
        printf("|       <<    [==]     >>      |\n");
        printf("|                              |\n");
        printf("|                              |\n");
        printf("|  [+] Daha Fazla              |\n");
        printf("|  [P] Oynatma Listesine Ekle  |\n");
        printf("|  [R] Şarkıya Puan Ver        |\n");
        printf("|  [D] Şarkıyı Sil             |\n");
        printf("|  [Q] Çıkış                   |\n");
        printf("|                              |\n");
        printf("+------------------------------+\n");
        printf("Seçiminizi yapın: ");

        char command;
        scanf(" %c", &command);

        switch (command) {
            case '>':
                if (currentPlaylist->head != NULL) {
                    currentPlaylist->head = currentPlaylist->head->next;
                }
                break;
            case '<':
                if (currentPlaylist->head != NULL) {
                    currentPlaylist->head = currentPlaylist->head->prev;
                }
                break;
            case 'D':
                deleteCurrentSong(currentPlaylist);
                break;
            case '+': {
                int choice;
                printf("\n[1] Şarkı Ekle\n");
                printf("[2] Oynatma Listelerini Görüntüle\n");
                printf("[3] Oynatma Listesi Oluştur\n");
                printf("[4] Şarkı Ara\n");
                printf("[5] Şarkıları Puanına Göre Listele\n");
                printf("[6] Oynatma Listesini Değiştir\n");
                printf("Seçiminiz: ");
                scanf("%d", &choice);
                if (choice == 1) {
                    char artist[100], name[100];
                    printf("Sanatçı adı: ");
                    scanf("%s", artist);
                    printf("Şarkı adı: ");
                    scanf("%s", name);
                    addSongToPlaylist(allSongsPlaylist, artist, name);
                } else if (choice == 2) {
                    PlaylistNode* playlist = selectPlaylist();
                    if (playlist) {
                        Song* temp = playlist->head;
                        if (temp == NULL) {
                            printf("Bu Oynatma Listesinde Şarkı Yok.\n");
                        } else {
                            printf("%s Oynatma Listesindeki Şarkılar:\n", playlist->name);
                            do {
                                printf("%s - %s\n", temp->artist, temp->name);
                                temp = temp->next;
                            } while (temp != playlist->head);
                        }
                    }
                } else if (choice == 3) {
                    char playlistName[100];
                    printf("Yeni Oynatma Listesinin Adı: ");
                    scanf("%s", playlistName);
                    addPlaylist(playlistName);
                } else if (choice == 4) {
                    searchSong(currentPlaylist);
                } else if (choice == 5) {
                    listRatedSongs();
                } else if (choice == 6) {
                    PlaylistNode* selectedPlaylist = selectPlaylist();
                    if (selectedPlaylist != NULL) {
                        currentPlaylist = selectedPlaylist;
                        printf("%s Oynatma Listesine Geçildi.\n", currentPlaylist->name);
                    }
                }
                break;
            }
            case 'P': {
                PlaylistNode* playlist = selectPlaylistExcludingAllSongs();
                if (playlist && strcmp(playlist->name, "Tüm Şarkılar") != 0 && currentPlaylist->head) {
                    addSongToPlaylist(playlist,
                        currentPlaylist->head->artist,
                        currentPlaylist->head->name);
                }
                break;
            }
            case 'R':
                if (currentPlaylist->head != NULL) {
                    int rating;
                    printf("Şarkıya 0-100 arasında puan verin: ");
                    scanf("%d", &rating);
                    
                    if (rating >= 0 && rating <= 100) {
                        addRating(
                            currentPlaylist->head->artist,
                            currentPlaylist->head->name,
                            rating
                        );
                    } else {
                        printf("Lütfen 0-100 arasında bir puan girin.\n");
                    }
                }
                break;
            case 'Q':
                printf("Çıkış yapılıyor...\n");
                return;
            default:
                printf("Geçersiz komut.\n");
                break;
        }
    }
}

int main() {
    // Tüm Şarkılar playlistini otomatik olarak oluşturuyor
    allSongsPlaylist = insertPlaylist(playlistRoot, "Tüm Şarkılar");
    playlistRoot = allSongsPlaylist;
    currentPlaylist = allSongsPlaylist;

    showMenu();
    return 0;
}
