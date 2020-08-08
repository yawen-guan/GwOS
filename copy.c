#include <stdio.h>
#include <string.h>

char img[512 * 2880];

int main() {
    FILE *f;
    // f = fopen("./command/helloworld", "rb");
    // fread(img, sizeof(char), 14132, f);
    // for (int i = 0; i < 14132; i ++)
    //     printf("%x ", img[i]);
    // fclose(f);

    f = fopen("GwOS.img", "rb");
    fseek(f, 300 * 512, SEEK_SET);
    fread(img, sizeof(char), 14132, f);
    for (int i = 0; i < 14132; i ++)
        printf("%x ", img[i]);
    fclose(f);

    // f = fopen("disk.img", "wb");
    // fseek(f, sizeof(char) * 512, SEEK_SET);
    // fwrite(img, sizeof(char), 512 * 2880, f);
    // fclose(f);
    return 0;
}