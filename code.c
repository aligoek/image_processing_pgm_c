#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>



// @param {const char*} dosyaAdi - Okunacak PGM dosyasinin adi
// @param {int*} w - resmin genisligi
// @param {int*} h - resmin yuksekligi
// @param {int*} mv - resmin max piksel degeri
// @returns {unsigned char**} - resim verilerini iceren 2 boyutlu dizi

// bu fonksiyon pgm dosyasini okur (p2 ve p5)

unsigned char **dosyaOku(const char *dosyaAdi, int *w, int *h, int *mv) {
    FILE *dosya = fopen(dosyaAdi, "rb");

    int i, j;

    int pikselDegeri;

    // pgm dosya turunu saklamak icin
    char tur[3];
    

    if (!dosya) {
        printf("Hata! Dosya acilamadi!");
        return NULL;
    }

    
    // dosyasnin ilk 2 karakterini oku
    fscanf(dosya, "%2s", tur);

    // p2 veya p5 mi kontrolu
    if (tur[0] != 'P' || (tur[1] != '5' && tur[1] != '2')) {

        printf("PGM dosyasi P2 ya da P5 olmali!");

        fclose(dosya);
        return NULL;
    }

    int ch;

    // yoru msatirlari ve bosluklari atla
    while (isspace(ch = fgetc(dosya)));
    if (ch == '#') {

        while (fgetc(dosya) != '\n');\

        while (isspace(ch = fgetc(dosya)));
    }

    ungetc(ch, dosya);

    // resmin genislik ve yuksekligini oku
    fscanf(dosya, "%d %d", w, h);

    while (isspace(ch = fgetc(dosya)));

    if (ch == '#') {
        while (fgetc(dosya) != '\n');
        while (isspace(ch = fgetc(dosya)));
    }

    ungetc(ch, dosya);

    // max value'yi oku
    fscanf(dosya, "%d", mv);

    unsigned char **resim = (unsigned char **)malloc(*h * sizeof(unsigned char *));

    if (!resim) {

        printf("Memory alocation hatasi!");
        fclose(dosya);
        return NULL;
    }

    for (i = 0; i < *h; i++) {
        resim[i] = (unsigned char *)malloc(*w * sizeof(unsigned char));
        if (!resim[i]) {
            printf("Memory alocation hatasi!");
            for (j = 0; j < i; j++) {
                free(resim[j]);
            }
            free(resim);
            fclose(dosya);

            return NULL;
        }
    }

    if (tur[1] == '5') {
        // P5 format (binary)
        for (i = 0; i < *h; i++) {
            fread(resim[i], 1, *w, dosya);
        }
    } else {
        // P2 format (ASCII)
        for (i = 0; i < *h; i++) {
            for (j = 0; j < *w; j++) {
                
                fscanf(dosya, "%d", &pikselDegeri);
                resim[i][j] = (unsigned char)pikselDegeri;
            }
        }
    }

    fclose(dosya);
    return resim;
}





// @param {const char*} dosyaAdi - Yazilacak PGM dosyasinin adi
// @param {unsigned char**} resim - Yazilacak resim verilerini iceren 2 boyutlu dizi
// @param {int} w - resmin genisligi
// @param {int} h - resmin yuksekligi
// @param {int} mv - resmin max piksel degeri
// @returns {void}

// bu fonksiyon resim verilerini P5 formatinda bir PGM dosyasina yazar.
void dosyaYaz(const char *dosyaAdi, unsigned char **resim, int w, int h, int mv) {
    // belirtilen dosya adiyla yazma modunda bir dosya pointeri olusturur.
    FILE *dosya = fopen(dosyaAdi, "wb");
    int i;

    // dosyanin basariyla acilip acilmadigini kontrol eder.
    if (!dosya) {
        printf("Hata! Dosya acilamadi!");
        return; // dosya acilamazsa fonksiyondan cikar.
    }

    // P5 formatinda (binary) PGM header'ini dosyaya yazar.
    fprintf(dosya, "P5\n%d %d\n%d\n", w, h, mv);
    // resim verilerinin her satirini dosyaya yazar.
    for (i = 0; i < h; i++) {
        fwrite(resim[i], 1, w, dosya);
    }

    
    fclose(dosya);
}




// @param {int} boyut - Olusturulacak Gauss kernelinin boyutu (tek sayi olmali)
// @param {double} sigma - Gauss fonksiyonunun standart sapmasi
// @returns {double**} - Olusturulan normalize edilmis Gauss kernelini iceren 2 boyutlu dizi

// bu fonksiyon verilen boyut ve sigma degeri ile normalize edilmis bir Gauss kerneli olusturur.
double **gaussKerneliOlustur(int boyut, double sigma) {
    double piSayisi = 3.14159265358979323846;
    int i, j;

    // kernel icin dinamik bellek ayirir.
    double **kernel = (double **)malloc(boyut * sizeof(double *));

    double ex;

    for (i = 0; i < boyut; i++) {
        kernel[i] = (double *)malloc(boyut * sizeof(double));
    }

    int merkez = boyut / 2;
    double toplam = 0.0;

    // Gauss fonksiyonunu kullanarak kernel degerlerini hesaplar.
    for (i = 0; i < boyut; i++) {
        for (j = 0; j < boyut; j++) {
            ex = -((i - merkez) * (i - merkez) + (j - merkez) * (j - merkez)) / (2.0 * sigma * sigma);
            kernel[i][j] = exp(ex) / (2.0 * piSayisi * sigma * sigma);
            toplam += kernel[i][j]; // kerneldeki tum degerlerin toplamini hesaplar
        }
    }

    // kerneli normalize eder.
    for (i = 0; i < boyut; i++) {
        for (j = 0; j < boyut; j++) {
            kernel[i][j] /= toplam; // her bir kernel degerini toplam degere boler
        }
    }

    return kernel; 
}


// @param {unsigned char**} resim - Filtrelenecek resim verilerini iceren 2 boyutlu dizi
// @param {int} w - resmin genisligi
// @param {int} h - resmin yuksekligi
// @param {int} boyut - Gauss kernelinin boyutu
// @param {double} sigma - Gauss fonksiyonunun standart sapmasi
// @returns {unsigned char**} - Gauss filtresi uygulanmis resim verilerini iceren 2 boyutlu dizi

// bu fonksiyon verilen resme Gauss filtresi uygular.
unsigned char **gaussFiltresi(unsigned char **resim, int w, int h, int boyut, double sigma) {

    int i, j, k, l;
    // Gauss kernelini olusturur.
    double **kernel = gaussKerneliOlustur(boyut, sigma);
    double toplam = 0.0;
    int tempX, tempY;

    // filtrelenmis resim icin dinamik bellek ayirir.
    unsigned char **filtreliResim = (unsigned char **)malloc(h * sizeof(unsigned char *));
    for (i = 0; i < h; i++) {
        filtreliResim[i] = (unsigned char *)malloc(w * sizeof(unsigned char));
    }

    int merkez = boyut / 2;

    // resimdeki her piksel icin filtreleme uygular.
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            toplam = 0.0;
            // kerneldeki her bir eleman icin komsuluk piksellerini gezer.
            for (k = 0; k < boyut; k++) {
                for (l = 0; l < boyut; l++) {
                    tempX = i - merkez + k;
                    tempY = j - merkez + l;

                    // sinirlar icinde olup olmadigini kontrol eder.
                    if (tempX >= 0 && tempX < h && tempY >= 0 && tempY < w) {
                        toplam += resim[tempX][tempY] * kernel[k][l]; // komsuluk piksel degeri ile kernel degerini carpar ve toplama ekler.
                    }
                }
            }
            // elde edilen toplam degeri 0-255 arasinda sinirlayarak filtrelenmis piksel degerini atar
            filtreliResim[i][j] = (unsigned char)(toplam > 255 ? 255 : (toplam < 0 ? 0 : toplam));
        }
    }

    // Kernel matrisini yazdirir 
    printf("Kernel boyutu: %d, Sigma: %.1f\n", boyut, sigma);
    for (i = 0; i < boyut; i++) {
        for (j = 0; j < boyut; j++) {
            printf("%lf ", kernel[i][j]);
        }
        printf("\n");
    }
    printf("\n");

    
    for (i = 0; i < boyut; i++) {
        free(kernel[i]);
    }
    free(kernel);

    return filtreliResim; 
}





// @param {unsigned char**} resim - Filtrelenecek resim verilerini iceren 2 boyutlu dizi
// @param {int} w - resmin genisligi
// @param {int} h - resmin yuksekligi
// @param {int[3][3]} kernel - Uygulanacak 3x3 Laplacian kerneli
// @returns {unsigned char**} - Laplacian filtresi uygulanmis resim verilerini iceren 2 boyutlu dizi, hata durumunda NULL

// bu fonksiyon verilen resme 3x3'luk bir Laplacian filtresi uygular.
unsigned char **laplacian(unsigned char **resim, int w, int h, int kernel[3][3]) {

    int i, j, k, l;
    double toplam = 0.0;
    int tempX, tempY;

    // filtrelenmis resim icin dinamik bellek ayirir
    unsigned char **filtreliResim = (unsigned char **)malloc(h * sizeof(unsigned char *));
    if (!filtreliResim)
    {
        printf("Hata! Bellek ayrilamadi!");
        return NULL;
    }

    for (i = 0; i < h; i++) {
        filtreliResim[i] = (unsigned char *)malloc(w * sizeof(unsigned char));
        if (!filtreliResim[i])
        {
            printf("Hata! Bellek ayrilamadi!");
            for(j = 0; j < i; j++)
                free(filtreliResim[j]);

            free(filtreliResim);

            return NULL;
        }
    }

    int merkez = 1; // 3x3 kernelin merkezi

    // resimdeki her piksel icin filtreleme uygular.
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            toplam = 0.0;
            // kerneldeki her bir eleman icin komsuluk piksellerini gezer (3x3 kernel)
            for (k = 0; k < 3; k++) {
                for (l = 0; l < 3; l++) {
                    tempX = i - merkez + k;
                    tempY = j - merkez + l;

                    // sinirlar icinde olup olmadigini kontrol eder.
                    if (tempX >= 0 && tempX < h && tempY >= 0 && tempY < w) {
                        toplam += resim[tempX][tempY] * kernel[k][l]; // komsuluk piksel degeri ile kernel degerini carpar ve toplama ekler
                    }
                }
            }
            // elde edilen toplam degeri 0-255 arasinda sinirlayarak filtrelenmis piksel degerini ata
            filtreliResim[i][j] = (unsigned char)(toplam > 255 ? 255 : (toplam < 0 ? 0 : toplam));
        }
    }
    return filtreliResim; 
}


int main() {
    const char *resimler[] = {"coins.ascii.pgm", "fruit.pgm", "saturn.ascii.pgm"};
    int resimSayisi = sizeof(resimler) / sizeof(resimler[0]);
    int boyutlar[] = {3, 5, 7};
    double sigmas[] = {1.0, 2.0, 4.0};
    int boyutSayisi = sizeof(boyutlar) / sizeof(boyutlar[0]);
    int sigmaSayisi = sizeof(sigmas) / sizeof(sigmas[0]);
    int i, j, k, l, b, s, f;

    int laplacianKernel1[3][3] = {{0, -1, 0}, {-1, 4, -1}, {0, -1, 0}};
    int laplacianKernel2[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};

    for (f = 0; f < resimSayisi; f++) {
        const char *dosyaAdi = resimler[f];

        int width, height, maxVal;

        unsigned char **resim = dosyaOku(dosyaAdi, &width, &height, &maxVal);

        if (!resim) {
            printf("Hata! Dosya acilamadi: %s\n", dosyaAdi);
        } else {
            
            unsigned char **laplacian1 = laplacian(resim, width, height, laplacianKernel1);
            if (laplacian1) {
                char outputAdi[200];
                const char *baseName = dosyaAdi;
                const char *dot = strrchr(dosyaAdi, '.');
                if (dot) {
                    int baseNameLen = dot - dosyaAdi;
                    strncpy(outputAdi, dosyaAdi, baseNameLen);
                    outputAdi[baseNameLen] = '\0';
                } else {
                    strcpy(outputAdi, dosyaAdi);
                }
                strcat(outputAdi, "_laplacian1_orjinal.pgm");
                dosyaYaz(outputAdi, laplacian1, width, height, maxVal);
                for (k = 0; k < height; k++) {
                    free(laplacian1[k]);
                }
                free(laplacian1);
            }

            unsigned char **laplacian2 = laplacian(resim, width, height, laplacianKernel2);
            if (laplacian2) {
                char outputAdi[200];
                const char *baseName = dosyaAdi;
                const char *dot = strrchr(dosyaAdi, '.');
                if (dot) {
                    int baseNameLen = dot - dosyaAdi;
                    strncpy(outputAdi, dosyaAdi, baseNameLen);
                    outputAdi[baseNameLen] = '\0';
                } else {
                    strcpy(outputAdi, dosyaAdi);
                }
                strcat(outputAdi, "_laplacian2_orjinal.pgm");
                dosyaYaz(outputAdi, laplacian2, width, height, maxVal);
                for (k = 0; k < height; k++) {
                    free(laplacian2[k]);
                }
                free(laplacian2);
            }

            for (i = 0; i < boyutSayisi; i++) {
                for (j = 0; j < sigmaSayisi; j++) {
                    unsigned char **filtreliResim = gaussFiltresi(resim, width, height, boyutlar[i], sigmas[j]);
                    if (filtreliResim == NULL) {
                        printf("Gauss filtresi uygulanamadi, hata.\n");
                    } else {
                        char outputAdi[200];
                        const char *baseName = dosyaAdi;
                        const char *dot = strrchr(dosyaAdi, '.');
                        if (dot) {
                            int baseNameLen = dot - dosyaAdi;
                            strncpy(outputAdi, dosyaAdi, baseNameLen);
                            outputAdi[baseNameLen] = '\0';
                        } else {
                            strcpy(outputAdi, dosyaAdi);
                        }
                        sprintf(outputAdi + strlen(outputAdi), "_filtre_%d_sigma_%.1f.pgm", boyutlar[i], sigmas[j]);
                        dosyaYaz(outputAdi, filtreliResim, width, height, maxVal);

                        
                        unsigned char **laplacianFiltreliResim1 = laplacian(filtreliResim, width, height, laplacianKernel1);
                        if (laplacianFiltreliResim1) {
                            if (dot) {
                                int baseNameLen = dot - dosyaAdi;
                                strncpy(outputAdi, dosyaAdi, baseNameLen);
                                outputAdi[baseNameLen] = '\0';
                            } else {
                                strcpy(outputAdi, dosyaAdi);
                            }
                            sprintf(outputAdi + strlen(outputAdi), "_laplacian1_filtre_%d_sigma_%.1f.pgm", boyutlar[i], sigmas[j]);
                            dosyaYaz(outputAdi, laplacianFiltreliResim1, width, height, maxVal);
                            for (k = 0; k < height; k++) {
                                free(laplacianFiltreliResim1[k]);
                            }
                            free(laplacianFiltreliResim1);
                        }

                        
                        unsigned char **laplacianFiltreliResim2 = laplacian(filtreliResim, width, height, laplacianKernel2);
                        if (laplacianFiltreliResim2) {
                            if (dot) {
                                int baseNameLen = dot - dosyaAdi;
                                strncpy(outputAdi, dosyaAdi, baseNameLen);
                                outputAdi[baseNameLen] = '\0';
                            } else {
                                strcpy(outputAdi, dosyaAdi);
                            }
                            sprintf(outputAdi + strlen(outputAdi), "_laplacian2_filtre_%d_sigma_%.1f.pgm", boyutlar[i], sigmas[j]);
                            dosyaYaz(outputAdi, laplacianFiltreliResim2, width, height, maxVal);
                            for (k = 0; k < height; k++) {
                                free(laplacianFiltreliResim2[k]);
                            }
                            free(laplacianFiltreliResim2);
                        }

                        
                        for (k = 0; k < height; k++) {
                            free(filtreliResim[k]);
                        }
                        free(filtreliResim);
                    }
                }
            }

            
            for (i = 0; i < height; i++) {
                free(resim[i]);
            }
            free(resim);
        }
    }

    return 0;
}


