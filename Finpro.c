#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <time.h>   
#include <ctype.h>  
#include <stdbool.h> 

typedef struct {
    char tanggal[12];
    int halamanSampai;
    char catatanSesi[1024]; 
} LogBacaanHarian;

typedef struct {
    char judul[100];
    char penulis[100];
    char bahasa[50];
    int tahunTerbit;
    int totalHalaman;    
    int halamanTerakhir; 
    
    char subkategoriFiksi[20];
    char comicType[20];
    int totalChapter;
    int chapterTerakhir;

    char kategori[20];
    
    char genres[5][50];
    int jumlahGenres; 

    int sudahDibaca;
    char tipeMedia[20];
    char pathFilePDF[256];
    
    LogBacaanHarian riwayatLog[50]; 
    int jumlahLog;
} Buku;

#define MAX_BUKU 100 
#define FILENAME "koleksi_buku.dat"

Buku koleksiBuku[MAX_BUKU];
int jumlahBuku = 0;

const char* FIKSI_GENRES[] = {"Fantasi", "Fiksi Ilmiah", "Horor", "Misteri", "Thriller", "Romansa", "Komedi", "Drama", "Petualangan", "Historis", "Young Adult", "Children's"};
const int NUM_FIKSI_GENRES = sizeof(FIKSI_GENRES) / sizeof(FIKSI_GENRES[0]);

const char* NONFIKSI_GENRES[] = {"Biografi", "Otokritik", "Sejarah", "Ilmiah Populer", "Self-Help", "Bisnis", "Memoir", "Jurnalistik", "Seni", "Agama & Spiritualitas"};
const int NUM_NONFIKSI_GENRES = sizeof(NONFIKSI_GENRES) / sizeof(NONFIKSI_GENRES[0]);

void tampilkanMenuUtama();
void tampilkanMenuAwal();
void tambahBuku();
void tampilkanSemuaBuku();
int pilihBukuDariDaftar();
void catatProgressBaca();
void editProgressBaca(); 
void hapusBuku();
void editBuku();
void tampilkanDetailBuku(int index);
void simpanData();
void muatData();
void bersihkanBufferInput(); 
void sortBukuByJudul();
const char* getStatusBacaBuku(const Buku* buku);
int cariBuku(const char* kriteria, const char* teksCari, int* hasilPencarianIndex);
void pilihGenresUntukBuku(Buku* buku);
void toLowerString(char* str);
void bersihkanData();
void bersihkanLayar();

void bersihkanBufferInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void bersihkanLayar() {
    #ifdef _WIN32
        system("cls"); 
    #else
        system("clear");
    #endif
}

void toLowerString(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

const char* getStatusBacaBuku(const Buku* buku) {
    if (strcmp(buku->subkategoriFiksi, "Komik") == 0) {
        if (buku->totalChapter > 0 && buku->chapterTerakhir == buku->totalChapter) {
            return "Sudah Dibaca (Komik)";
        } else if (buku->chapterTerakhir > 0) {
            return "Ongoing (Komik)";
        } else {
            return "Belum Dibaca (Komik)";
        }
    } else {
        if (buku->totalHalaman > 0 && buku->halamanTerakhir == buku->totalHalaman) {
            return "Sudah Dibaca";
        } else if (buku->halamanTerakhir > 0) {
            return "Belum Selesai Dibaca";
        } else {
            return "Belum Dibaca";
        }
    }
}

void pilihGenresUntukBuku(Buku* buku) {
    bersihkanLayar();
    int pilihanGenre;
    buku->jumlahGenres = 0;
    char inputGenreManual[50];

    printf("\n--- Pilih Genre untuk Buku Ini ---\n");
    printf("0. Selesai Memilih Genre\n");

    const char** currentGenres;
    int numCurrentGenres;

    if (strcmp(buku->kategori, "Fiksi") == 0) {
        currentGenres = FIKSI_GENRES;
        numCurrentGenres = NUM_FIKSI_GENRES;
        printf("Genre Fiksi yang tersedia:\n");
    } else {
        currentGenres = NONFIKSI_GENRES;
        numCurrentGenres = NUM_NONFIKSI_GENRES;
        printf("Genre Non-Fiksi yang tersedia:\n");
    }

    for (int i = 0; i < numCurrentGenres; ++i) {
        printf("%d. %s\n", i + 1, currentGenres[i]);
    }
    printf("%d. Masukkan Genre Lain Secara Manual\n", numCurrentGenres + 1);

    while (buku->jumlahGenres < 5) {
        printf("Pilih nomor genre (atau 0 untuk selesai, %d untuk manual) [%d/5]: ", 
               numCurrentGenres + 1, buku->jumlahGenres, 5);
        scanf("%d", &pilihanGenre);
        
        // Panggil bersihkanBufferInput setelah setiap scanf
        bersihkanBufferInput(); 

        if (pilihanGenre == 0) {
            break;
        } else if (pilihanGenre >= 1 && pilihanGenre <= numCurrentGenres) {
            bool isDuplicate = false;
            for(int i = 0; i < buku->jumlahGenres; i++) {
                if(strcmp(buku->genres[i], currentGenres[pilihanGenre - 1]) == 0) {
                    isDuplicate = true;
                    break;
                }
            }
            if(isDuplicate) {
                printf("Genre '%s' sudah ditambahkan.\n", currentGenres[pilihanGenre - 1]);
            } else {
                strcpy(buku->genres[buku->jumlahGenres], currentGenres[pilihanGenre - 1]);
                buku->jumlahGenres++;
                printf("Genre '%s' ditambahkan.\n", currentGenres[pilihanGenre - 1]);
            }
        } else if (pilihanGenre == numCurrentGenres + 1) {
            printf("Masukkan nama genre manual (maks %d karakter): ", 50 - 1);
            fgets(inputGenreManual, sizeof(inputGenreManual), stdin);
            inputGenreManual[strcspn(inputGenreManual, "\n")] = 0;

            if (strlen(inputGenreManual) > 0) {
                bool isDuplicate = false;
                for(int i = 0; i < buku->jumlahGenres; i++) {
                    if(strcmp(buku->genres[i], inputGenreManual) == 0) {
                        isDuplicate = true;
                        break;
                    }
                }
                if(isDuplicate) {
                    printf("Genre '%s' sudah ditambahkan.\n", inputGenreManual);
                } else {
                    strcpy(buku->genres[buku->jumlahGenres], inputGenreManual);
                    buku->jumlahGenres++;
                    printf("Genre '%s' ditambahkan.\n", inputGenreManual);
                }
            } else {
                printf("Input genre manual kosong.\n");
            }
        } else {
            printf("Pilihan tidak valid.\n");
        }
    }
    if (buku->jumlahGenres == 0) {
        strcpy(buku->genres[0], "Tidak Ada Genre");
        buku->jumlahGenres = 1;
    }
}

int cariBuku(const char* kriteria, const char* teksCari, int* hasilPencarianIndex) {
    int count = 0;
    char lowerTeksCari[100];
    strcpy(lowerTeksCari, teksCari);
    toLowerString(lowerTeksCari);

    for (int i = 0; i < jumlahBuku; i++) {
        char targetText[100];
        bool isMatch = false;

        if (strcmp(kriteria, "judul") == 0) {
            strcpy(targetText, koleksiBuku[i].judul);
            toLowerString(targetText);
            if (strstr(targetText, lowerTeksCari) != NULL) isMatch = true;
        } else if (strcmp(kriteria, "penulis") == 0) {
            strcpy(targetText, koleksiBuku[i].penulis);
            toLowerString(targetText);
            if (strstr(targetText, lowerTeksCari) != NULL) isMatch = true;
        } else if (strcmp(kriteria, "bahasa") == 0) {
            strcpy(targetText, koleksiBuku[i].bahasa);
            toLowerString(targetText);
            if (strstr(targetText, lowerTeksCari) != NULL) isMatch = true;
        } else if (strcmp(kriteria, "kategori") == 0) {
            strcpy(targetText, koleksiBuku[i].kategori);
            toLowerString(targetText);
            if (strstr(targetText, lowerTeksCari) != NULL) isMatch = true;
        } else if (strcmp(kriteria, "subkategori") == 0) {
            strcpy(targetText, koleksiBuku[i].subkategoriFiksi);
            toLowerString(targetText);
            if (strstr(targetText, lowerTeksCari) != NULL) isMatch = true;
        } else if (strcmp(kriteria, "comicType") == 0) {
            strcpy(targetText, koleksiBuku[i].comicType);
            toLowerString(targetText);
            if (strstr(targetText, lowerTeksCari) != NULL) isMatch = true;
        } else if (strcmp(kriteria, "genre") == 0) {
            for (int g = 0; g < koleksiBuku[i].jumlahGenres; g++) {
                strcpy(targetText, koleksiBuku[i].genres[g]);
                toLowerString(targetText);
                if (strstr(targetText, lowerTeksCari) != NULL) {
                    isMatch = true;
                    break;
                }
            }
        } else if (strcmp(kriteria, "tipeMedia") == 0) {
            strcpy(targetText, koleksiBuku[i].tipeMedia);
            toLowerString(targetText);
            if (strstr(targetText, lowerTeksCari) != NULL) isMatch = true;
        }

        if (isMatch) {
            hasilPencarianIndex[count++] = i;
        }
    }
    return count;
}

void tampilkanMenuUtama() {
    bersihkanLayar();
    printf("\n\n");
    printf("------------------------------------------\n");
    printf("         P U S T A K A R A               \n");
    printf("------------------------------------------\n");
    printf("\n");
    printf("    1. Tambah Buku Baru                   \n");
    printf("    2. Tampilkan Daftar Buku              \n");
    printf("    3. Pilih/Cari Buku dari Daftar        \n");
    printf("    4. Catat Kemajuan Membaca             \n");
    printf("    5. Edit Data Buku                     \n");
    printf("    6. Edit Progres Bacaan (Halaman/Catatan)\n");
    printf("    7. Hapus Buku                         \n");
    printf("    8. Bersihkan Semua Data Koleksi       \n");
    printf("    0. Kembali ke Menu Awal               \n");
    printf("\n");
    printf("------------------------------------------\n");
    printf("\n");
    printf("Pilih opsi: ");
}

void tampilkanMenuAwal() {
    bersihkanLayar();
    printf("\n\n");
    printf("------------------------------------------\n");
    printf("    Selamat Datang di PustaKara!          \n");
    printf("------------------------------------------\n");
    printf("\n\n");
    printf("    1. Mulai                              \n");
    printf("    0. Keluar                             \n");
    printf("\n\n");
    printf("------------------------------------------\n");
    printf("\n");
    printf("Pilih opsi: ");
}

void tambahBuku() {
    bersihkanLayar();
    if (jumlahBuku >= MAX_BUKU) {
        printf("Koleksi buku penuh. Tidak bisa menambah buku lagi.\n");
        return;
    }

    printf("\n--- Tambah Buku Baru --- \n");
    printf("Judul Buku: ");
    char tempJudul[100];
    // Perbaikan utama: Menggunakan getchar() setelah scanf di main untuk membuang newline.
    // Di sini, kita asumsikan input string pertama (judul) sudah bersih.
    // Jika masih ada masalah, bisa tambahkan getchar() manual di awal fungsi ini
    // atau sesuaikan bersihkanBufferInput() lebih agresif.
    fgets(tempJudul, sizeof(tempJudul), stdin);
    tempJudul[strcspn(tempJudul, "\n")] = 0;
    strcpy(koleksiBuku[jumlahBuku].judul, tempJudul);

    printf("Penulis: ");
    char tempPenulis[100];
    fgets(tempPenulis, sizeof(tempPenulis), stdin);
    tempPenulis[strcspn(tempPenulis, "\n")] = 0;
    strcpy(koleksiBuku[jumlahBuku].penulis, tempPenulis);

    printf("Bahasa (contoh: Inggris, Jepang, Prancis): ");
    char tempBahasa[50];
    fgets(tempBahasa, sizeof(tempBahasa), stdin);
    tempBahasa[strcspn(tempBahasa, "\n")] = 0;
    strcpy(koleksiBuku[jumlahBuku].bahasa, tempBahasa);

    printf("Tahun Terbit: ");
    scanf("%d", &koleksiBuku[jumlahBuku].tahunTerbit);
    bersihkanBufferInput(); // Perlu bersihkan setelah scanf

    int tipeMediaPilihan;
    printf("Pilih Tipe Media:\n1. Fisik\n2. Non-Fisik (Digital/E-book)\nPilihan: ");
    scanf("%d", &tipeMediaPilihan);
    bersihkanBufferInput();
    if (tipeMediaPilihan == 1) {
        strcpy(koleksiBuku[jumlahBuku].tipeMedia, "Fisik");
        strcpy(koleksiBuku[jumlahBuku].pathFilePDF, "None");
    } else {
        strcpy(koleksiBuku[jumlahBuku].tipeMedia, "Non-Fisik");
        printf("Path File PDF (contoh: C:\\buku\\namabuku.pdf, tekan Enter jika tidak ada): ");
        char tempPathPDF[256];
        fgets(tempPathPDF, sizeof(tempPathPDF), stdin);
        tempPathPDF[strcspn(tempPathPDF, "\n")] = 0;
        strcpy(koleksiBuku[jumlahBuku].pathFilePDF, tempPathPDF);
        if (strlen(koleksiBuku[jumlahBuku].pathFilePDF) == 0) {
            strcpy(koleksiBuku[jumlahBuku].pathFilePDF, "None");
        }
    }

    int kategoriPilihan;
    printf("Pilih Kategori:\n1. Fiksi\n2. Non-Fiksi\nPilihan: ");
    scanf("%d", &kategoriPilihan);
    bersihkanBufferInput();

    if (kategoriPilihan == 1) {
        strcpy(koleksiBuku[jumlahBuku].kategori, "Fiksi");
        
        int subkategoriPilihan;
        printf("Pilih Subkategori Fiksi:\n1. Novel\n2. Komik\n3. Antologi\nPilihan: ");
        scanf("%d", &subkategoriPilihan);
        bersihkanBufferInput();

        if (subkategoriPilihan == 1) {
            strcpy(koleksiBuku[jumlahBuku].subkategoriFiksi, "Novel");
            printf("Total Halaman Buku: ");
            scanf("%d", &koleksiBuku[jumlahBuku].totalHalaman);
            bersihkanBufferInput();
            koleksiBuku[jumlahBuku].chapterTerakhir = 0;
            koleksiBuku[jumlahBuku].totalChapter = 0;
            strcpy(koleksiBuku[jumlahBuku].comicType, "None");
        } else if (subkategoriPilihan == 2) {
            strcpy(koleksiBuku[jumlahBuku].subkategoriFiksi, "Komik");
            koleksiBuku[jumlahBuku].totalHalaman = 0;

            int comicTypePilihan;
            printf("Pilih Jenis Komik:\n1. Comics(Amerika)\n2. Manga(Jepang)\n3. Manhwa(Korea)\n4. Manhua(Cina)\nPilihan: ");
            scanf("%d", &comicTypePilihan);
            bersihkanBufferInput();
            if (comicTypePilihan == 1) strcpy(koleksiBuku[jumlahBuku].comicType, "Comics(Amerika)");
            else if (comicTypePilihan == 2) strcpy(koleksiBuku[jumlahBuku].comicType, "Manga(Jepang)");
            else if (comicTypePilihan == 3) strcpy(koleksiBuku[jumlahBuku].comicType, "Manhwa(Korea)");
            else if (comicTypePilihan == 4) strcpy(koleksiBuku[jumlahBuku].comicType, "Manhua(Cina)");
            else strcpy(koleksiBuku[jumlahBuku].comicType, "None");

            int completeStatus;
            printf("Apakah komik ini sudah selesai (Complete)?\n1. Ya\n2. Tidak (Ongoing)\nPilihan: ");
            scanf("%d", &completeStatus);
            bersihkanBufferInput();

            if (completeStatus == 1) {
                printf("Jumlah Total Chapter: ");
                scanf("%d", &koleksiBuku[jumlahBuku].totalChapter);
                bersihkanBufferInput();
            } else {
                koleksiBuku[jumlahBuku].totalChapter = 0;
            }
            printf("Chapter Terakhir Dibaca: ");
            scanf("%d", &koleksiBuku[jumlahBuku].chapterTerakhir);
            bersihkanBufferInput();

        } else if (subkategoriPilihan == 3) {
            strcpy(koleksiBuku[jumlahBuku].subkategoriFiksi, "Antologi");
            printf("Total Halaman Buku: ");
            scanf("%d", &koleksiBuku[jumlahBuku].totalHalaman);
            bersihkanBufferInput();
            koleksiBuku[jumlahBuku].chapterTerakhir = 0;
            koleksiBuku[jumlahBuku].totalChapter = 0;
            strcpy(koleksiBuku[jumlahBuku].comicType, "None");
        } else {
            strcpy(koleksiBuku[jumlahBuku].subkategoriFiksi, "Lainnya");
            printf("Total Halaman Buku: ");
            scanf("%d", &koleksiBuku[jumlahBuku].totalHalaman);
            bersihkanBufferInput();
            koleksiBuku[jumlahBuku].chapterTerakhir = 0;
            koleksiBuku[jumlahBuku].totalChapter = 0;
            strcpy(koleksiBuku[jumlahBuku].comicType, "None");
        }
    } else {
        strcpy(koleksiBuku[jumlahBuku].kategori, "Non-Fiksi");
        strcpy(koleksiBuku[jumlahBuku].subkategoriFiksi, "None");
        strcpy(koleksiBuku[jumlahBuku].comicType, "None");
        koleksiBuku[jumlahBuku].totalChapter = 0;
        koleksiBuku[jumlahBuku].chapterTerakhir = 0;
        printf("Total Halaman Buku: ");
        scanf("%d", &koleksiBuku[jumlahBuku].totalHalaman);
        bersihkanBufferInput();
    }

    pilihGenresUntukBuku(&koleksiBuku[jumlahBuku]);

    koleksiBuku[jumlahBuku].halamanTerakhir = 0;
    koleksiBuku[jumlahBuku].sudahDibaca = 0;
    koleksiBuku[jumlahBuku].jumlahLog = 0;

    jumlahBuku++;
    printf("Buku berhasil ditambahkan!\n");
    simpanData();
}

void tampilkanSemuaBuku() {
    bersihkanLayar();
    printf("\n--- Koleksi Buku Anda (%d Buku) ---\n", jumlahBuku);
    if (jumlahBuku == 0) {
        printf("Belum ada buku dalam koleksi.\n");
        return;
    }

    sortBukuByJudul(); 

    for (int i = 0; i < jumlahBuku; ++i) {
        printf("\n[%d] Judul: %s\n", i + 1, koleksiBuku[i].judul);
        printf("    Penulis: %s\n", koleksiBuku[i].penulis);
        printf("    Bahasa: %s\n", koleksiBuku[i].bahasa);
        printf("    Kategori: %s", koleksiBuku[i].kategori);
        printf(" | Tipe Media: %s\n", koleksiBuku[i].tipeMedia);
        if (strcmp(koleksiBuku[i].kategori, "Fiksi") == 0) {
            printf("    Subkategori: %s\n", koleksiBuku[i].subkategoriFiksi);
            if (strcmp(koleksiBuku[i].subkategoriFiksi, "Komik") == 0) {
                printf("    Jenis Komik: %s\n", koleksiBuku[i].comicType);
                printf("    Progress: Chapter %d", koleksiBuku[i].chapterTerakhir);
                if (koleksiBuku[i].totalChapter > 0) {
                    printf(" dari %d\n", koleksiBuku[i].totalChapter);
                } else {
                    printf(" (Ongoing)\n");
                }
            } else {
                printf("    Halaman: %d/%d\n", koleksiBuku[i].halamanTerakhir, koleksiBuku[i].totalHalaman);
            }
        } else {
            printf("    Halaman: %d/%d\n", koleksiBuku[i].halamanTerakhir, koleksiBuku[i].totalHalaman);
        }
        printf("    Genre: ");
        for (int g = 0; g < koleksiBuku[i].jumlahGenres; g++) {
            printf("%s%s", koleksiBuku[i].genres[g], (g == koleksiBuku[i].jumlahGenres - 1) ? "" : ", ");
        }
        printf("\n");
        printf("    Status: %s\n", getStatusBacaBuku(&koleksiBuku[i]));
    }
    printf("\nUntuk detail riwayat baca dan review final, gunakan opsi 'Pilih/Cari Buku dari Daftar'.\n");
}

void tampilkanDetailBuku(int index) {
    bersihkanLayar();
    if (index < 0 || index >= jumlahBuku) {
        printf("Indeks buku tidak valid.\n");
        return;
    }
    printf("\n--- Detail Buku [%d] ---\n", index + 1);
    printf("Judul: %s\n", koleksiBuku[index].judul);
    printf("Penulis: %s\n", koleksiBuku[index].penulis);
    printf("Bahasa: %s\n", koleksiBuku[index].bahasa);
    printf("Tahun Terbit: %d\n", koleksiBuku[index].tahunTerbit);
    printf("Kategori: %s\n", koleksiBuku[index].kategori);
    printf("Tipe Media: %s\n", koleksiBuku[index].tipeMedia);
    if (strcmp(koleksiBuku[index].tipeMedia, "Non-Fisik") == 0 && strlen(koleksiBuku[index].pathFilePDF) > 0 && strcmp(koleksiBuku[index].pathFilePDF, "None") != 0) {
        printf("Lokasi File PDF: %s\n", koleksiBuku[index].pathFilePDF);
    }

    if (strcmp(koleksiBuku[index].kategori, "Fiksi") == 0) {
        printf("Subkategori: %s\n", koleksiBuku[index].subkategoriFiksi);
        if (strcmp(koleksiBuku[index].subkategoriFiksi, "Komik") == 0) {
            printf("Jenis Komik: %s\n", koleksiBuku[index].comicType);
            printf("Progress: Chapter %d", koleksiBuku[index].chapterTerakhir);
            if (koleksiBuku[index].totalChapter > 0) {
                printf(" dari %d\n", koleksiBuku[index].totalChapter);
            } else {
                printf(" (Ongoing)\n");
            }
        } else {
            printf("Total Halaman: %d\n", koleksiBuku[index].totalHalaman);
            printf("Halaman Terakhir Dibaca: %d\n", koleksiBuku[index].halamanTerakhir);
        }
    } else {
        printf("Total Halaman: %d\n", koleksiBuku[index].totalHalaman);
        printf("Halaman Terakhir Dibaca: %d\n", koleksiBuku[index].halamanTerakhir);
    }
    
    printf("Status: %s\n", getStatusBacaBuku(&koleksiBuku[index]));
    printf("Genre: ");
    for (int g = 0; g < koleksiBuku[index].jumlahGenres; g++) {
        printf("%s%s", koleksiBuku[index].genres[g], (g == koleksiBuku[index].jumlahGenres - 1) ? "" : ", ");
    }
    printf("\n");

    printf("\n--- Riwayat Bacaan per Tanggal ---\n");
    if (koleksiBuku[index].jumlahLog == 0) {
        printf("Belum ada riwayat bacaan untuk buku ini.\n");
    } else {
        for (int i = 0; i < koleksiBuku[index].jumlahLog; i++) {
            printf("[%d] Tanggal: %s | ", i + 1, koleksiBuku[index].riwayatLog[i].tanggal);
            if (strcmp(koleksiBuku[index].subkategoriFiksi, "Komik") == 0) {
                 printf("Chapter Dicapai: %d\n", koleksiBuku[index].riwayatLog[i].halamanSampai);
            } else {
                 printf("Halaman Dicapai: %d\n", koleksiBuku[index].riwayatLog[i].halamanSampai);
            }
            
            printf("    Catatan Sesi: %s\n", koleksiBuku[index].riwayatLog[i].catatanSesi);
            
            if (strcmp(koleksiBuku[index].subkategoriFiksi, "Komik") == 0) {
                if (koleksiBuku[index].totalChapter > 0 && koleksiBuku[index].riwayatLog[i].halamanSampai == koleksiBuku[index].totalChapter) {
                    printf("    *** Ini adalah Review Final Komik! ***\n");
                }
            } else {
                if (koleksiBuku[index].totalHalaman > 0 && koleksiBuku[index].riwayatLog[i].halamanSampai == koleksiBuku[index].totalHalaman) {
                    printf("    *** Ini adalah Review Final Buku! ***\n");
                }
            }
        }
    }
}

int pilihBukuDariDaftar() {
    bersihkanLayar();
    if (jumlahBuku == 0) {
        printf("Belum ada buku dalam koleksi untuk dipilih.\n");
        return -1;
    }
    
    int pilihanFilter;
    char teksCari[100];
    int hasilPencarianIndex[MAX_BUKU];
    int jumlahHasil = 0;

    printf("\n--- Pilih/Cari Buku dari Daftar ---\n");
    printf("1. Tampilkan semua buku (urut abjad Judul)\n");
    printf("2. Cari berdasarkan Judul\n");
    printf("3. Cari berdasarkan Penulis\n");
    printf("4. Cari berdasarkan Bahasa\n");
    printf("5. Cari berdasarkan Kategori (Fiksi/Non-Fiksi)\n");
    printf("6. Cari berdasarkan Subkategori Fiksi (Novel, Komik, Antologi)\n");
    printf("7. Cari berdasarkan Jenis Komik (Manga, Manhwa, dll)\n");
    printf("8. Cari berdasarkan Genre\n");
    printf("9. Cari berdasarkan Tipe Media (Fisik/Non-Fisik)\n");
    printf("Pilih opsi pencarian/tampilan: ");
    scanf("%d", &pilihanFilter);
    bersihkanBufferInput();

    if (pilihanFilter == 1) {
        sortBukuByJudul();
        for (int i = 0; i < jumlahBuku; i++) {
            hasilPencarianIndex[jumlahHasil++] = i;
        }
        if (jumlahHasil == 0) {
            return -1;
        }
    } else if (pilihanFilter >= 2 && pilihanFilter <= 9) {
        printf("Masukkan teks yang ingin dicari: ");
        fgets(teksCari, sizeof(teksCari), stdin);
        teksCari[strcspn(teksCari, "\n")] = 0;

        const char* kriteriaCari;
        if (pilihanFilter == 2) kriteriaCari = "judul";
        else if (pilihanFilter == 3) kriteriaCari = "penulis";
        else if (pilihanFilter == 4) kriteriaCari = "bahasa";
        else if (pilihanFilter == 5) kriteriaCari = "kategori";
        else if (pilihanFilter == 6) kriteriaCari = "subkategori";
        else if (pilihanFilter == 7) kriteriaCari = "comicType";
        else if (pilihanFilter == 8) kriteriaCari = "genre";
        else kriteriaCari = "tipeMedia";

        jumlahHasil = cariBuku(kriteriaCari, teksCari, hasilPencarianIndex);
        if (jumlahHasil == 0) {
            printf("Tidak ada buku yang cocok dengan kriteria tersebut.\n");
            return -1;
        }
    } else {
        printf("Opsi tidak valid.\n");
        return -1;
    }

    const int BUKU_PER_HALAMAN = 5;
    int totalHalaman = (jumlahHasil + BUKU_PER_HALAMAN - 1) / BUKU_PER_HALAMAN;
    int halamanSekarang = 1;
    int pilihanNavigasi;
    int selectedOriginalIndex = -1;

    do {
        bersihkanLayar();
        printf("\n--- Hasil Pencarian/Daftar Buku (%d ditemukan) ---\n", jumlahHasil);
        printf("          Halaman %d dari %d\n", halamanSekarang, totalHalaman);
        printf("------------------------------------------\n");

        int startIndex = (halamanSekarang - 1) * BUKU_PER_HALAMAN;
        int endIndex = startIndex + BUKU_PER_HALAMAN;
        if (endIndex > jumlahHasil) {
            endIndex = jumlahHasil;
        }

        if (jumlahHasil == 0) {
            printf("Tidak ada buku di halaman ini.\n");
        } else {
            for (int i = startIndex; i < endIndex; ++i) {
                int originalIndex = hasilPencarianIndex[i];
                printf("[%d] Judul: %s (Penulis: %s)\n", 
                       i + 1, koleksiBuku[originalIndex].judul, koleksiBuku[originalIndex].penulis);
                printf("    Kategori: %s", koleksiBuku[originalIndex].kategori);
                printf(" | Tipe Media: %s\n", koleksiBuku[originalIndex].tipeMedia);
                if (strcmp(koleksiBuku[originalIndex].kategori, "Fiksi") == 0) {
                    printf(" | Subkategori: %s", koleksiBuku[originalIndex].subkategoriFiksi);
                    if (strcmp(koleksiBuku[originalIndex].subkategoriFiksi, "Komik") == 0) {
                        printf(" | Jenis Komik: %s", koleksiBuku[originalIndex].comicType);
                    }
                }
                printf(" | Bahasa: %s\n", koleksiBuku[originalIndex].bahasa);
                printf("    Genre: ");
                for(int g = 0; g < koleksiBuku[originalIndex].jumlahGenres; ++g) {
                    printf("%s%s", koleksiBuku[originalIndex].genres[g], (g == koleksiBuku[originalIndex].jumlahGenres - 1) ? "" : ", ");
                }
                printf("\n");
            }
        }
        
        printf("------------------------------------------\n");
        printf("Navigasi: [0] Kembali ke Menu Pencarian | ");
        if (halamanSekarang > 1) {
            printf("[8] Sebelumnya | ");
        }
        if (halamanSekarang < totalHalaman) {
            printf("[9] Selanjutnya | ");
        }
        printf("Pilih buku (1-%d): ", jumlahHasil);

        scanf("%d", &pilihanNavigasi);
        bersihkanBufferInput();

        if (pilihanNavigasi == 8 && halamanSekarang > 1) {
            halamanSekarang--;
        } else if (pilihanNavigasi == 9 && halamanSekarang < totalHalaman) {
            halamanSekarang++;
        } else if (pilihanNavigasi == 0) {
            return -1;
        } else if (pilihanNavigasi >= 1 && pilihanNavigasi <= jumlahHasil) {
            selectedOriginalIndex = hasilPencarianIndex[pilihanNavigasi - 1];
            break;
        } else {
            printf("Pilihan tidak valid. Silakan coba lagi.\n");
            printf("\nTekan ENTER untuk melanjutkan...\n");
            bersihkanBufferInput();
        }
    } while (true);

    return selectedOriginalIndex;
}

void catatProgressBaca() {
    bersihkanLayar();
    int indexBuku = pilihBukuDariDaftar();

    if (indexBuku != -1) {
        bersihkanLayar(); 
        printf("\n--- Catat Kemajuan Membaca & Ringkasan Sesi Baru --- \n");
        printf("Buku yang dipilih: %s\n", koleksiBuku[indexBuku].judul);
        
        int newProgressValue;
        int currentProgressValue;
        int totalValue;
        const char* unitName;

        if (strcmp(koleksiBuku[indexBuku].subkategoriFiksi, "Komik") == 0) {
            currentProgressValue = koleksiBuku[indexBuku].chapterTerakhir;
            totalValue = koleksiBuku[indexBuku].totalChapter;
            unitName = "Chapter";
        } else {
            currentProgressValue = koleksiBuku[indexBuku].halamanTerakhir;
            totalValue = koleksiBuku[indexBuku].totalHalaman;
            unitName = "Halaman";
        }

        printf("%s terakhir yang tercatat: %d", unitName, currentProgressValue);
        if (totalValue > 0) {
            printf(" dari %d\n", totalValue);
        } else {
            printf(" (Ongoing)\n");
        }
        
        printf("Masukkan %s terakhir yang Anda capai di sesi ini: ", unitName);
        scanf("%d", &newProgressValue);
        bersihkanBufferInput();

        if (newProgressValue < 0 || (totalValue > 0 && newProgressValue > totalValue)) {
            printf("Input %s tidak valid. Mohon masukkan angka antara 0 dan %d.\n", unitName, totalValue);
            return;
        }
        if (newProgressValue < currentProgressValue && currentProgressValue != 0) {
             printf("%s yang dicapai tidak boleh kurang dari progress terakhir (%d).\n", unitName, currentProgressValue);
             return;
        }
        
        if (strcmp(koleksiBuku[indexBuku].subkategoriFiksi, "Komik") == 0) {
            koleksiBuku[indexBuku].chapterTerakhir = newProgressValue;
        } else {
            koleksiBuku[indexBuku].halamanTerakhir = newProgressValue;
        }

        bool isFinished = false;
        if (totalValue > 0 && newProgressValue == totalValue) {
            isFinished = true;
        }

        LogBacaanHarian newLog;
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        strftime(newLog.tanggal, sizeof(newLog.tanggal), "%Y-%m-%d", t);

        newLog.halamanSampai = newProgressValue;

        if (isFinished) {
            koleksiBuku[indexBuku].sudahDibaca = 1; 
            printf("Selamat! Anda telah menyelesaikan %s ini.\n", (strcmp(koleksiBuku[indexBuku].subkategoriFiksi, "Komik") == 0) ? "komik" : "buku");
            printf("Tulis review final Anda untuk %s ini (maks %d karakter):\n", (strcmp(koleksiBuku[indexBuku].subkategoriFiksi, "Komik") == 0) ? "komik" : "buku", 1024 - 1);
        } else {
            koleksiBuku[indexBuku].sudahDibaca = 0; 
            printf("Tulis ringkasan/catatan singkat Anda untuk sesi bacaan ini (maks %d karakter):\n", 1024 - 1);
        }
        
        fgets(newLog.catatanSesi, sizeof(newLog.catatanSesi), stdin);
        newLog.catatanSesi[strcspn(newLog.catatanSesi, "\n")] = 0;

        koleksiBuku[indexBuku].riwayatLog[koleksiBuku[indexBuku].jumlahLog] = newLog;
        koleksiBuku[indexBuku].jumlahLog++;

        printf("Kemajuan dan catatan sesi berhasil dicatat!\n");
        simpanData();
    }
}

void editProgressBaca() { 
    bersihkanLayar();
    int indexBuku = pilihBukuDariDaftar();

    if (indexBuku != -1) {
        if (koleksiBuku[indexBuku].jumlahLog == 0) {
            printf("Belum ada riwayat bacaan untuk buku ini yang bisa diedit.\n");
            return;
        }

        bersihkanLayar(); 
        printf("\n--- Edit Progres Bacaan untuk Buku: %s ---\n", koleksiBuku[indexBuku].judul);
        printf("Riwayat Bacaan yang Tersedia:\n");
        for (int i = 0; i < koleksiBuku[indexBuku].jumlahLog; ++i) {
            printf("[%d] Tanggal: %s | Halaman/Chapter Dicapai: %d | Catatan: %s\n",
                   i + 1, koleksiBuku[indexBuku].riwayatLog[i].tanggal,
                   koleksiBuku[indexBuku].riwayatLog[i].halamanSampai,
                   koleksiBuku[indexBuku].riwayatLog[i].catatanSesi);
        }

        int pilihanLog;
        printf("Pilih nomor riwayat bacaan yang ingin diedit (1-%d, atau 0 untuk batal): ", koleksiBuku[indexBuku].jumlahLog);
        scanf("%d", &pilihanLog);
        bersihkanBufferInput();

        if (pilihanLog == 0) {
            printf("Edit progres bacaan dibatalkan.\n");
            return;
        }
        if (pilihanLog < 1 || pilihanLog > koleksiBuku[indexBuku].jumlahLog) {
            printf("Pilihan riwayat tidak valid.\n");
            return;
        }

        int logIndex = pilihanLog - 1;

        printf("\n--- Mengedit Log Bacaan [%d] --- \n", pilihanLog);
        printf("1. Ubah Halaman/Chapter Dicapai (saat ini: %d)\n", koleksiBuku[indexBuku].riwayatLog[logIndex].halamanSampai);
        printf("2. Ubah Catatan Sesi (saat ini: %s)\n", koleksiBuku[indexBuku].riwayatLog[logIndex].catatanSesi);
        printf("0. Batalkan Edit Log Ini\n");
        printf("Pilihan: ");

        int pilihanEditLog;
        scanf("%d", &pilihanEditLog);
        bersihkanBufferInput();

        switch (pilihanEditLog) {
            case 1: {
                int newProgressValue;
                const char* unitName = (strcmp(koleksiBuku[indexBuku].subkategoriFiksi, "Komik") == 0) ? "Chapter" : "Halaman";
                int totalValue = (strcmp(koleksiBuku[indexBuku].subkategoriFiksi, "Komik") == 0) ? koleksiBuku[indexBuku].totalChapter : koleksiBuku[indexBuku].totalHalaman;

                printf("Masukkan %s baru (saat ini: %d): ", unitName, koleksiBuku[indexBuku].riwayatLog[logIndex].halamanSampai);
                scanf("%d", &newProgressValue);
                bersihkanBufferInput();

                if (newProgressValue < 0 || (totalValue > 0 && newProgressValue > totalValue)) {
                    printf("Input %s tidak valid. Mohon masukkan angka antara 0 dan %d.\n", unitName, totalValue);
                    printf("Perubahan dibatalkan.\n");
                    return;
                }
                
                koleksiBuku[indexBuku].riwayatLog[logIndex].halamanSampai = newProgressValue;

                int maxProgress = 0;
                for(int i = 0; i < koleksiBuku[indexBuku].jumlahLog; ++i) {
                    if (koleksiBuku[indexBuku].riwayatLog[i].halamanSampai > maxProgress) {
                        maxProgress = koleksiBuku[indexBuku].riwayatLog[i].halamanSampai;
                    }
                }

                if (strcmp(koleksiBuku[indexBuku].subkategoriFiksi, "Komik") == 0) {
                    koleksiBuku[indexBuku].chapterTerakhir = maxProgress;
                } else {
                    koleksiBuku[indexBuku].halamanTerakhir = maxProgress;
                }
                
                if (totalValue > 0 && maxProgress == totalValue) {
                    koleksiBuku[indexBuku].sudahDibaca = 1;
                    printf("Buku/Komik ini sekarang ditandai selesai.\n");
                } else {
                    koleksiBuku[indexBuku].sudahDibaca = 0;
                    printf("Buku/Komik ini sekarang ditandai belum selesai.\n");
                }

                printf("Halaman/Chapter berhasil diubah.\n");
                break;
            }
            case 2: {
                printf("Masukkan catatan sesi baru (maks %d karakter, saat ini: %s): \n", 1024 - 1, koleksiBuku[indexBuku].riwayatLog[logIndex].catatanSesi);
                char tempCatatan[1024];
                fgets(tempCatatan, sizeof(tempCatatan), stdin);
                tempCatatan[strcspn(tempCatatan, "\n")] = 0;
                strcpy(koleksiBuku[indexBuku].riwayatLog[logIndex].catatanSesi, tempCatatan);
                printf("Catatan sesi berhasil diubah.\n");
                break;
            }
            case 0:
                printf("Edit log bacaan dibatalkan.\n");
                return;
            default:
                printf("Pilihan tidak valid. Edit log bacaan dibatalkan.\n");
                return;
        }
        simpanData();
    }
}

void editBuku() {
    bersihkanLayar();
    int indexBuku = pilihBukuDariDaftar();

    if (indexBuku != -1) {
        printf("\n--- Edit Data Buku: %s ---\n", koleksiBuku[indexBuku].judul);
        printf("Pilih field yang ingin diubah:\n");
        printf("1. Judul\n");
        printf("2. Penulis\n");
        printf("3. Bahasa\n");
        printf("4. Tahun Terbit\n");
        printf("5. Tipe Media (Fisik/Non-Fisik) & Path PDF\n");
        printf("6. Kategori & Subkategori (Fiksi/Non-Fiksi/Novel/Komik/Antologi/Jenis Komik)\n");
        printf("7. Genre\n");
        printf("8. Total Halaman/Chapter (hanya untuk komik complete/buku biasa)\n");
        printf("0. Batalkan Edit\n");
        printf("Pilihan: ");

        int pilihanEdit;
        scanf("%d", &pilihanEdit);
        bersihkanBufferInput();

        switch (pilihanEdit) {
            case 1:
                printf("Judul baru: ");
                fgets(koleksiBuku[indexBuku].judul, sizeof(koleksiBuku[indexBuku].judul), stdin);
                koleksiBuku[indexBuku].judul[strcspn(koleksiBuku[indexBuku].judul, "\n")] = 0;
                printf("Judul berhasil diubah.\n");
                break;
            case 2:
                printf("Penulis baru: ");
                fgets(koleksiBuku[indexBuku].penulis, sizeof(koleksiBuku[indexBuku].penulis), stdin);
                koleksiBuku[indexBuku].penulis[strcspn(koleksiBuku[indexBuku].penulis, "\n")] = 0;
                printf("Penulis berhasil diubah.\n");
                break;
            case 3:
                printf("Bahasa baru: ");
                fgets(koleksiBuku[indexBuku].bahasa, sizeof(koleksiBuku[indexBuku].bahasa), stdin);
                koleksiBuku[indexBuku].bahasa[strcspn(koleksiBuku[indexBuku].bahasa, "\n")] = 0;
                printf("Bahasa berhasil diubah.\n");
                break;
            case 4:
                printf("Tahun Terbit baru: ");
                scanf("%d", &koleksiBuku[indexBuku].tahunTerbit);
                bersihkanBufferInput();
                printf("Tahun Terbit berhasil diubah.\n");
                break;
            case 5: {
                int tipeMediaPilihan;
                printf("Tipe Media saat ini: %s\n", koleksiBuku[indexBuku].tipeMedia);
                printf("Pilih Tipe Media Baru:\n1. Fisik\n2. Non-Fisik (Digital/E-book)\nPilihan: ");
                scanf("%d", &tipeMediaPilihan);
                bersihkanBufferInput();
                if (tipeMediaPilihan == 1) {
                    strcpy(koleksiBuku[indexBuku].tipeMedia, "Fisik");
                    strcpy(koleksiBuku[indexBuku].pathFilePDF, "None");
                    printf("Tipe media diubah menjadi Fisik. Path PDF dihapus.\n");
                } else if (tipeMediaPilihan == 2) {
                    strcpy(koleksiBuku[indexBuku].tipeMedia, "Non-Fisik");
                    printf("Path File PDF saat ini: %s\n", koleksiBuku[indexBuku].pathFilePDF);
                    printf("Path File PDF baru (tekan Enter jika tidak ada perubahan/kosongkan): ");
                    fgets(koleksiBuku[indexBuku].pathFilePDF, sizeof(koleksiBuku[indexBuku].pathFilePDF), stdin);
                    koleksiBuku[indexBuku].pathFilePDF[strcspn(koleksiBuku[indexBuku].pathFilePDF, "\n")] = 0;
                    if (strlen(koleksiBuku[indexBuku].pathFilePDF) == 0) {
                        strcpy(koleksiBuku[indexBuku].pathFilePDF, "None");
                    }
                    printf("Tipe media diubah menjadi Non-Fisik.\n");
                } else {
                    printf("Pilihan tidak valid. Perubahan dibatalkan.\n");
                }
                break;
            }
            case 6: {
                int kategoriPilihan;
                printf("Kategori saat ini: %s\n", koleksiBuku[indexBuku].kategori);
                printf("Pilih Kategori Baru:\n1. Fiksi\n2. Non-Fiksi\nPilihan: ");
                scanf("%d", &kategoriPilihan);
                bersihkanBufferInput();

                if (kategoriPilihan == 1) {
                    strcpy(koleksiBuku[indexBuku].kategori, "Fiksi");
                    int subkategoriPilihan;
                    printf("Subkategori Fiksi saat ini: %s\n", koleksiBuku[indexBuku].subkategoriFiksi);
                    printf("Pilih Subkategori Fiksi Baru:\n1. Novel\n2. Komik\n3. Antologi\nPilihan: ");
                    scanf("%d", &subkategoriPilihan);
                    bersihkanBufferInput();

                    if (subkategoriPilihan == 1) {
                        strcpy(koleksiBuku[indexBuku].subkategoriFiksi, "Novel");
                        koleksiBuku[indexBuku].totalChapter = 0; koleksiBuku[indexBuku].chapterTerakhir = 0; strcpy(koleksiBuku[indexBuku].comicType, "None");
                    } else if (subkategoriPilihan == 2) {
                        strcpy(koleksiBuku[indexBuku].subkategoriFiksi, "Komik");
                        koleksiBuku[indexBuku].totalHalaman = 0;

                        int comicTypePilihan;
                        printf("Jenis Komik saat ini: %s\n", koleksiBuku[indexBuku].comicType);
                        printf("Pilih Jenis Komik Baru:\n1. Comics(Amerika)\n2. Manga(Jepang)\n3. Manhwa(Korea)\n4. Manhua(Cina)\nPilihan: ");
                        scanf("%d", &comicTypePilihan);
                        bersihkanBufferInput();
                        if (comicTypePilihan == 1) strcpy(koleksiBuku[indexBuku].comicType, "Comics(Amerika)");
                        else if (comicTypePilihan == 2) strcpy(koleksiBuku[indexBuku].comicType, "Manga(Jepang)");
                        else if (comicTypePilihan == 3) strcpy(koleksiBuku[indexBuku].comicType, "Manhwa(Korea)");
                        else if (comicTypePilihan == 4) strcpy(koleksiBuku[indexBuku].comicType, "Manhua(Cina)");
                        else strcpy(koleksiBuku[indexBuku].comicType, "None");

                        int completeStatus;
                        printf("Status Komik saat ini: %s (Total Chapter: %d)\n", (koleksiBuku[indexBuku].totalChapter > 0) ? "Complete" : "Ongoing", koleksiBuku[indexBuku].totalChapter);
                        printf("Apakah komik ini sudah selesai (Complete)?\n1. Ya\n2. Tidak (Ongoing)\nPilihan: ");
                        scanf("%d", &completeStatus);
                        bersihkanBufferInput();
                        if (completeStatus == 1) {
                            printf("Jumlah Total Chapter baru: ");
                            scanf("%d", &koleksiBuku[indexBuku].totalChapter);
                            bersihkanBufferInput();
                        } else {
                            koleksiBuku[indexBuku].totalChapter = 0;
                        }
                    } else if (subkategoriPilihan == 3) {
                        strcpy(koleksiBuku[indexBuku].subkategoriFiksi, "Antologi");
                        koleksiBuku[indexBuku].totalChapter = 0; koleksiBuku[indexBuku].chapterTerakhir = 0; strcpy(koleksiBuku[indexBuku].comicType, "None");
                    } else {
                        printf("Pilihan subkategori tidak valid. Perubahan dibatalkan.\n");
                    }
                    printf("Kategori dan Subkategori berhasil diubah.\n");
                } else if (kategoriPilihan == 2) {
                    strcpy(koleksiBuku[indexBuku].kategori, "Non-Fiksi");
                    strcpy(koleksiBuku[indexBuku].subkategoriFiksi, "None");
                    strcpy(koleksiBuku[indexBuku].comicType, "None");
                    koleksiBuku[indexBuku].totalChapter = 0; koleksiBuku[indexBuku].chapterTerakhir = 0;
                    printf("Kategori diubah menjadi Non-Fiksi.\n");
                } else {
                    printf("Pilihan tidak valid. Perubahan dibatalkan.\n");
                }
                break;
            }
            case 7:
                pilihGenresUntukBuku(&koleksiBuku[indexBuku]);
                printf("Genre berhasil diubah.\n");
                break;
            case 8:
                if (strcmp(koleksiBuku[indexBuku].subkategoriFiksi, "Komik") == 0) {
                    printf("Total Chapter saat ini: %d\n", koleksiBuku[indexBuku].totalChapter);
                    printf("Jumlah Total Chapter baru (0 jika Ongoing): ");
                    scanf("%d", &koleksiBuku[indexBuku].totalChapter);
                    bersihkanBufferInput();
                    printf("Total Chapter berhasil diubah.\n");
                } else {
                    printf("Total Halaman saat ini: %d\n", koleksiBuku[indexBuku].totalHalaman);
                    printf("Total Halaman baru: ");
                    scanf("%d", &koleksiBuku[indexBuku].totalHalaman);
                    bersihkanBufferInput();
                    printf("Total Halaman berhasil diubah.\n");
                }
                break;
            case 0:
                printf("Edit data dibatalkan.\n");
                break;
            default:
                printf("Pilihan tidak valid. Edit dibatalkan.\n");
                break;
        }
        simpanData();
    }
}

void hapusBuku() {
    bersihkanLayar();
    int indexHapus = pilihBukuDariDaftar();

    if (indexHapus != -1) {
        printf("Anda yakin ingin menghapus buku '%s'? (y/n): ", koleksiBuku[indexHapus].judul);
        char konfirmasi;
        scanf("%c", &konfirmasi);
        bersihkanBufferInput();

        if (konfirmasi == 'y' || konfirmasi == 'Y') {
            for (int i = indexHapus; i < jumlahBuku - 1; i++) {
                koleksiBuku[i] = koleksiBuku[i + 1];
            }
            jumlahBuku--;
            printf("Buku berhasil dihapus!\n");
            simpanData();
        } else {
            printf("Penghapusan dibatalkan.\n");
        }
    }
}

void bersihkanData() {
    bersihkanLayar();
    char konfirmasi1[10];
    char konfirmasi2[10];
    char konfirmasi3[10];

    printf("\n--- PERINGATAN! PENGHAPUSAN DATA KOLEKSI ---\n");
    printf("ANDA YAKIN? Tindakan ini akan menghapus SEMUA data buku Anda secara permanen. (ketik 'YA' untuk yakin): ");
    fgets(konfirmasi1, sizeof(konfirmasi1), stdin);
    konfirmasi1[strcspn(konfirmasi1, "\n")] = 0;
    toLowerString(konfirmasi1);

    if (strcmp(konfirmasi1, "ya") == 0) {
        printf("\nYAKIN NIH? Setelah ini, tidak ada jalan kembali. (ketik 'YA' untuk yakin nih): ");
        fgets(konfirmasi2, sizeof(konfirmasi2), stdin);
        konfirmasi2[strcspn(konfirmasi2, "\n")] = 0;
        toLowerString(konfirmasi2);

        if (strcmp(konfirmasi2, "ya") == 0) {
            printf("\nEHH YAKIN BANGET? Ini adalah kesempatan terakhir Anda! (ketik 'YA' untuk yakin banget): ");
            fgets(konfirmasi3, sizeof(konfirmasi3), stdin);
            konfirmasi3[strcspn(konfirmasi3, "\n")] = 0;
            toLowerString(konfirmasi3);

            if (strcmp(konfirmasi3, "ya") == 0) {
                if (remove(FILENAME) == 0) {
                    jumlahBuku = 0; 
                    printf("\nSEMUA DATA KOLEKSI BUKU BERHASIL DIHAPUS PERMANEN!\n");
                    printf("Program akan direset. Silakan restart program jika ingin memulai ulang.\n");
                    exit(0);
                } else {
                    printf("\nGagal menghapus file data. Mungkin file tidak ditemukan atau ada masalah izin.\n");
                }
            } else {
                printf("\nPenghapusan semua data DIBATALKAN. Data Anda aman.\n");
            }
        } else {
            printf("\nPenghapusan semua data DIBATALKAN. Data Anda aman.\n");
        }
    } else {
        printf("\nPenghapusan semua data DIBATALKAN. Data Anda aman.\n");
    }
}

void simpanData() {
    FILE *file = fopen(FILENAME, "wb");
    if (file == NULL) {
        printf("Error: Gagal membuka file untuk menyimpan data.\n");
        return;
    }
    
    fwrite(&jumlahBuku, sizeof(int), 1, file);
    fwrite(koleksiBuku, sizeof(Buku), jumlahBuku, file);
    
    fclose(file);
    printf("Data berhasil disimpan ke %s.\n", FILENAME);
}

void muatData() {
    FILE *file = fopen(FILENAME, "rb");
    if (file == NULL) {
        printf("Belum ada data buku yang tersimpan. Memulai koleksi baru.\n");
        jumlahBuku = 0;
        return;
    }
    
    fread(&jumlahBuku, sizeof(int), 1, file);
    fread(koleksiBuku, sizeof(Buku), jumlahBuku, file);

    fclose(file);
    printf("Data berhasil dimuat dari %s.\n", FILENAME);
}

void sortBukuByJudul() {
    if (jumlahBuku <= 1) return;

    Buku tempBuku;

    for (int i = 0; i < jumlahBuku - 1; i++) {
        for (int j = 0; j < jumlahBuku - 1 - i; j++) {
            #ifdef _WIN32 
            if (_stricmp(koleksiBuku[j].judul, koleksiBuku[j+1].judul) > 0) {
            #else 
            if (strcasecmp(koleksiBuku[j].judul, koleksiBuku[j+1].judul) > 0) {
            #endif
                tempBuku = koleksiBuku[j];
                koleksiBuku[j] = koleksiBuku[j+1];
                koleksiBuku[j+1] = tempBuku;
            }
        }
    }
}

int main() {
    int pilihanMenuAwal;
    
    muatData();

    do {
        tampilkanMenuAwal();
        scanf("%d", &pilihanMenuAwal);
        bersihkanBufferInput();

        if (pilihanMenuAwal == 1) {
            int pilihanUtama;
            int selectedBookIndex;
            do {
                tampilkanMenuUtama();
                scanf("%d", &pilihanUtama);
                bersihkanBufferInput();

                switch (pilihanUtama) {
                    case 1:
                        tambahBuku();
                        break;
                    case 2:
                        tampilkanSemuaBuku();
                        break;
                    case 3:
                        selectedBookIndex = pilihBukuDariDaftar();
                        if (selectedBookIndex != -1) {
                            tampilkanDetailBuku(selectedBookIndex);
                        }
                        break;
                    case 4:
                        catatProgressBaca();
                        break;
                    case 5:
                        editBuku();
                        break;
                    case 6:
                        editProgressBaca();
                        break;
                    case 7:
                        hapusBuku();
                        break;
                    case 8:
                        bersihkanData();
                        break;
                    case 0:
                        printf("Kembali ke Menu Awal.\n");
                        break;
                    default:
                        printf("Pilihan tidak valid. Silakan coba lagi.\n");
                }
                if (pilihanUtama != 0) {
                    printf("\nTekan ENTER untuk melanjutkan...\n");
                    bersihkanBufferInput();
                }
            } while (pilihanUtama != 0);
        } else if (pilihanMenuAwal == 0) {
            printf("Terima kasih telah menggunakan PustaKara. Sampai jumpa!\n");
        } else {
            printf("Pilihan tidak valid. Silakan coba lagi.\n");
        }
    } while (pilihanMenuAwal != 0);

    return 0;
}