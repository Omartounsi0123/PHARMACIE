#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_MEDICATIONS 200
#define MAX_SUPPLIERS 3
#define FIELD_SIZE 100
#define MAX_LINE_LENGTH 1200
#define DATA_FILE "TxtMed.txt"
#define TAX_RATE 0.19f

typedef struct {
    int day;
    int month;
    int year;
} Date;

typedef struct {
    char name[FIELD_SIZE];
    char address[FIELD_SIZE];
    char phone[FIELD_SIZE];
} Supplier;

typedef struct {
    char reference[FIELD_SIZE];
    Date manufacturingDate;
    Date expirationDate;
} Lot;

typedef struct {
    int number;
    char name[FIELD_SIZE];
    char laboratory[FIELD_SIZE];
    Lot lot;
    float price;
    int quantityInStock;
    Supplier suppliers[MAX_SUPPLIERS];
    int supplierCount;
    int storageSector;
} Medication;

void clearInputBuffer(void);
void readLine(const char *prompt, char *buffer, size_t size);
int readInt(const char *prompt);
float readFloat(const char *prompt);
Date readDate(const char *label);
int isValidDate(Date date);
int compareDates(Date a, Date b);
int isExpired(Date date);
void safeStringCopy(char *dest, const char *src, size_t size);
int parseIntToken(const char *token, int defaultValue);
float parseFloatToken(const char *token, float defaultValue);
int findMedicationByNumber(const Medication meds[], int count, int number);
int findMedicationByLotReference(const Medication meds[], int count, const char *reference);
int containsIgnoreCase(const char *text, const char *pattern);
void printDate(Date date);
void printMedication(const Medication *med);
void addMedication(Medication meds[], int *count);
void searchMedicationByNumber(const Medication meds[], int count);
void displayAllMedicationsBySector(const Medication meds[], int count);
void displayMedicationsByModel(const Medication meds[], int count);
void displayAllSuppliers(const Medication meds[], int count);
void displayAllMedicationsByOrderedDate(const Medication meds[], int count);
void placeOrder(Medication meds[], int count);
void deleteMedicationByBatchReference(Medication meds[], int *count);
void removeMedicationAt(Medication meds[], int *count, int index);
void saveData(const Medication meds[], int count);
void loadData(Medication meds[], int *count);
void showMenu(void);

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

void readLine(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, (int)size, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
}

int readInt(const char *prompt) {
    int value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1) {
            clearInputBuffer();
            return value;
        }
        clearInputBuffer();
        printf("Valeur invalide. Veuillez saisir un entier.\n");
    }
}

float readFloat(const char *prompt) {
    float value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%f", &value) == 1) {
            clearInputBuffer();
            return value;
        }
        clearInputBuffer();
        printf("Valeur invalide. Veuillez saisir un nombre.\n");
    }
}

Date readDate(const char *label) {
    Date date;
    while (1) {
        printf("%s (jj mm aaaa): ", label);
        if (scanf("%d %d %d", &date.day, &date.month, &date.year) == 3) {
            clearInputBuffer();
            if (isValidDate(date)) {
                return date;
            }
            printf("Date invalide.\n");
        } else {
            clearInputBuffer();
            printf("Saisie invalide.\n");
        }
    }
}

int isValidDate(Date date) {
    int daysInMonth;
    int isLeapYear;

    if (date.year < 1900 || date.month < 1 || date.month > 12 || date.day < 1) {
        return 0;
    }

    isLeapYear = (date.year % 400 == 0) || (date.year % 4 == 0 && date.year % 100 != 0);
    switch (date.month) {
        case 2:
            daysInMonth = isLeapYear ? 29 : 28;
            break;
        case 4:
        case 6:
        case 9:
        case 11:
            daysInMonth = 30;
            break;
        default:
            daysInMonth = 31;
    }

    return date.day <= daysInMonth;
}

int compareDates(Date a, Date b) {
    if (a.year != b.year) {
        return (a.year < b.year) ? -1 : 1;
    }
    if (a.month != b.month) {
        return (a.month < b.month) ? -1 : 1;
    }
    if (a.day != b.day) {
        return (a.day < b.day) ? -1 : 1;
    }
    return 0;
}

int isExpired(Date date) {
    if (!isValidDate(date)) {
        return 0;
    }

    time_t now = time(NULL);
    struct tm *today = localtime(&now);
    Date current = {today->tm_mday, today->tm_mon + 1, today->tm_year + 1900};
    return compareDates(date, current) < 0;
}

void safeStringCopy(char *dest, const char *src, size_t size) {
    strncpy(dest, src ? src : "", size - 1);
    dest[size - 1] = '\0';
}

int parseIntToken(const char *token, int defaultValue) {
    char *endPtr;
    long value;

    if (token == NULL) {
        return defaultValue;
    }

    value = strtol(token, &endPtr, 10);
    if (endPtr == token) {
        return defaultValue;
    }

    return (int)value;
}

float parseFloatToken(const char *token, float defaultValue) {
    char *endPtr;
    float value;

    if (token == NULL) {
        return defaultValue;
    }

    value = strtof(token, &endPtr);
    if (endPtr == token) {
        return defaultValue;
    }

    return value;
}

int findMedicationByNumber(const Medication meds[], int count, int number) {
    int i;
    for (i = 0; i < count; i++) {
        if (meds[i].number == number) {
            return i;
        }
    }
    return -1;
}

int findMedicationByLotReference(const Medication meds[], int count, const char *reference) {
    int i;
    for (i = 0; i < count; i++) {
        if (strcmp(meds[i].lot.reference, reference) == 0) {
            return i;
        }
    }
    return -1;
}

int containsIgnoreCase(const char *text, const char *pattern) {
    size_t i, j;
    size_t textLen = strlen(text);
    size_t patternLen = strlen(pattern);

    if (patternLen == 0) {
        return 1;
    }

    if (patternLen > textLen) {
        return 0;
    }

    for (i = 0; i <= textLen - patternLen; i++) {
        int match = 1;
        for (j = 0; j < patternLen; j++) {
            if (tolower((unsigned char)text[i + j]) != tolower((unsigned char)pattern[j])) {
                match = 0;
                break;
            }
        }
        if (match) {
            return 1;
        }
    }

    return 0;
}

void printDate(Date date) {
    if (!isValidDate(date)) {
        printf("N/A");
        return;
    }
    printf("%02d/%02d/%04d", date.day, date.month, date.year);
}

void printMedication(const Medication *med) {
    int i;
    printf("\nNumero: %d\n", med->number);
    printf("Nom: %s\n", med->name);
    printf("Laboratoire: %s\n", med->laboratory);
    printf("Lot reference: %s\n", strlen(med->lot.reference) == 0 ? "N/A" : med->lot.reference);
    printf("Date fabrication: ");
    printDate(med->lot.manufacturingDate);
    printf("\nDate peremption: ");
    printDate(med->lot.expirationDate);
    printf("\nPrix: %.3f\n", med->price);
    printf("Quantite en stock: %d\n", med->quantityInStock);
    printf("Secteur stockage: %d\n", med->storageSector);
    printf("Fournisseurs (%d):\n", med->supplierCount);

    for (i = 0; i < med->supplierCount; i++) {
        printf("  - %s | %s | %s\n", med->suppliers[i].name, med->suppliers[i].address, med->suppliers[i].phone);
    }
}

void addMedication(Medication meds[], int *count) {
    Medication med;
    int i;

    if (*count >= MAX_MEDICATIONS) {
        printf("Stock plein. Impossible d'ajouter un medicament.\n");
        return;
    }

    med.number = readInt("Numero du medicament: ");
    if (findMedicationByNumber(meds, *count, med.number) != -1) {
        printf("Un medicament avec ce numero existe deja.\n");
        return;
    }

    readLine("Nom: ", med.name, sizeof(med.name));
    readLine("Laboratoire: ", med.laboratory, sizeof(med.laboratory));

    readLine("Reference lot (laisser vide si non initialisee): ", med.lot.reference, sizeof(med.lot.reference));
    if (strlen(med.lot.reference) == 0) {
        med.lot.manufacturingDate = (Date){0, 0, 0};
        med.lot.expirationDate = (Date){0, 0, 0};
    } else {
        med.lot.manufacturingDate = readDate("Date fabrication");
        med.lot.expirationDate = readDate("Date peremption");
    }

    med.price = readFloat("Prix: ");
    med.quantityInStock = readInt("Quantite en stock: ");

    while (1) {
        med.storageSector = readInt("Secteur stockage (1-5): ");
        if (med.storageSector >= 1 && med.storageSector <= 5) {
            break;
        }
        printf("Secteur invalide, choisir entre 1 et 5.\n");
    }

    while (1) {
        med.supplierCount = readInt("Nombre de fournisseurs (0-3): ");
        if (med.supplierCount >= 0 && med.supplierCount <= MAX_SUPPLIERS) {
            break;
        }
        printf("Nombre invalide.\n");
    }

    for (i = 0; i < med.supplierCount; i++) {
        printf("Fournisseur %d:\n", i + 1);
        readLine("  Nom: ", med.suppliers[i].name, sizeof(med.suppliers[i].name));
        readLine("  Adresse: ", med.suppliers[i].address, sizeof(med.suppliers[i].address));
        readLine("  Telephone: ", med.suppliers[i].phone, sizeof(med.suppliers[i].phone));
    }

    meds[*count] = med;
    (*count)++;
    printf("Medicament ajoute avec succes.\n");
}

void searchMedicationByNumber(const Medication meds[], int count) {
    int number = readInt("Numero a rechercher: ");
    int index = findMedicationByNumber(meds, count, number);

    if (index == -1) {
        printf("Medicament introuvable.\n");
        return;
    }

    printMedication(&meds[index]);
}

void displayAllMedicationsBySector(const Medication meds[], int count) {
    int sector;
    int i;

    if (count == 0) {
        printf("Aucun medicament en stock.\n");
        return;
    }

    for (sector = 1; sector <= 5; sector++) {
        int found = 0;
        printf("\n=== Secteur %d ===\n", sector);
        for (i = 0; i < count; i++) {
            if (meds[i].storageSector == sector) {
                found = 1;
                printMedication(&meds[i]);
            }
        }
        if (!found) {
            printf("Aucun medicament dans ce secteur.\n");
        }
    }
}

void displayMedicationsByModel(const Medication meds[], int count) {
    char model[FIELD_SIZE];
    int i;
    int found = 0;

    if (count == 0) {
        printf("Aucun medicament en stock.\n");
        return;
    }

    readLine("Entrer le modele (nom ou laboratoire): ", model, sizeof(model));

    for (i = 0; i < count; i++) {
        if (containsIgnoreCase(meds[i].name, model) || containsIgnoreCase(meds[i].laboratory, model)) {
            printMedication(&meds[i]);
            found = 1;
        }
    }

    if (!found) {
        printf("Aucun medicament correspondant au modele.\n");
    }
}

void displayAllSuppliers(const Medication meds[], int count) {
    int i;
    int j;

    if (count == 0) {
        printf("Aucun medicament en stock.\n");
        return;
    }

    for (i = 0; i < count; i++) {
        printf("\nMedicament #%d - %s\n", meds[i].number, meds[i].name);
        if (meds[i].supplierCount == 0) {
            printf("  Aucun fournisseur.\n");
            continue;
        }
        for (j = 0; j < meds[i].supplierCount; j++) {
            printf("  Fournisseur %d: %s | %s | %s\n", j + 1, meds[i].suppliers[j].name,
                   meds[i].suppliers[j].address, meds[i].suppliers[j].phone);
        }
    }
}

void displayAllMedicationsByOrderedDate(const Medication meds[], int count) {
    int indices[MAX_MEDICATIONS];
    int i, j;

    if (count == 0) {
        printf("Aucun medicament en stock.\n");
        return;
    }

    for (i = 0; i < count; i++) {
        indices[i] = i;
    }

    for (i = 0; i < count - 1; i++) {
        for (j = i + 1; j < count; j++) {
            const Medication *a = &meds[indices[i]];
            const Medication *b = &meds[indices[j]];
            int swap = 0;

            if (a->storageSector > b->storageSector) {
                swap = 1;
            } else if (a->storageSector == b->storageSector &&
                       compareDates(a->lot.manufacturingDate, b->lot.manufacturingDate) > 0) {
                swap = 1;
            }

            if (swap) {
                int tmp = indices[i];
                indices[i] = indices[j];
                indices[j] = tmp;
            }
        }
    }

    printf("\nMedicaments classes par secteur puis date de fabrication:\n");
    for (i = 0; i < count; i++) {
        const Medication *med = &meds[indices[i]];
        printf("Secteur %d | Lot %s | Fabrication ", med->storageSector,
               strlen(med->lot.reference) ? med->lot.reference : "N/A");
        printDate(med->lot.manufacturingDate);
        printf(" | Medicament %d - %s | Quantite %d\n", med->number, med->name, med->quantityInStock);
    }
}

void placeOrder(Medication meds[], int count) {
    int number;
    int requested;
    int index;
    char confirm[10];
    float subtotal;
    float totalWithTax;

    if (count == 0) {
        printf("Aucun medicament en stock.\n");
        return;
    }

    number = readInt("Numero du medicament a commander: ");
    index = findMedicationByNumber(meds, count, number);
    if (index == -1) {
        printf("Medicament introuvable.\n");
        return;
    }

    if (strlen(meds[index].lot.reference) == 0) {
        printf("Aucun lot disponible pour ce medicament.\n");
        return;
    }

    requested = readInt("Quantite demandee: ");
    if (requested <= 0) {
        printf("Quantite invalide.\n");
        return;
    }

    if (requested > meds[index].quantityInStock) {
        printf("Stock insuffisant. Quantite disponible: %d\n", meds[index].quantityInStock);
        return;
    }

    subtotal = requested * meds[index].price;
    totalWithTax = subtotal * (1.0f + TAX_RATE);

    printf("\nFacture:\n");
    printf("Medicament: %s\n", meds[index].name);
    printf("Quantite: %d\n", requested);
    printf("Sous-total: %.3f\n", subtotal);
    printf("Total TTC (%.0f%%): %.3f\n", TAX_RATE * 100.0f, totalWithTax);

    readLine("Valider la facture ? (o/n): ", confirm, sizeof(confirm));
    if (tolower((unsigned char)confirm[0]) == 'o') {
        meds[index].quantityInStock -= requested;
        printf("Commande validee. Nouveau stock: %d\n", meds[index].quantityInStock);
    } else {
        printf("Commande annulee.\n");
    }
}

void removeMedicationAt(Medication meds[], int *count, int index) {
    int i;
    for (i = index; i < *count - 1; i++) {
        meds[i] = meds[i + 1];
    }
    (*count)--;
}

void deleteMedicationByBatchReference(Medication meds[], int *count) {
    char reference[FIELD_SIZE];
    int index;

    if (*count == 0) {
        printf("Aucun medicament en stock.\n");
        return;
    }

    readLine("Reference lot a supprimer: ", reference, sizeof(reference));
    index = findMedicationByLotReference(meds, *count, reference);
    if (index == -1) {
        printf("Aucun medicament pour cette reference lot.\n");
        return;
    }

    if (meds[index].quantityInStock <= 0 || isExpired(meds[index].lot.expirationDate)) {
        removeMedicationAt(meds, count, index);
        printf("Medicament supprime automatiquement (quantite nulle ou perime).\n");
        return;
    }

    printf("Le lot contient encore %d unites.\n", meds[index].quantityInStock);
    {
        char confirm[10];
        readLine("Confirmer suppression ? (o/n): ", confirm, sizeof(confirm));
        if (tolower((unsigned char)confirm[0]) == 'o') {
            removeMedicationAt(meds, count, index);
            printf("Medicament supprime.\n");
        } else {
            printf("Suppression annulee.\n");
        }
    }
}

void saveData(const Medication meds[], int count) {
    FILE *file = fopen(DATA_FILE, "w");
    int i, j;

    if (file == NULL) {
        printf("Erreur d'ouverture du fichier de sauvegarde.\n");
        return;
    }

    fprintf(file, "%d\n", count);
    for (i = 0; i < count; i++) {
        const Medication *m = &meds[i];
        fprintf(file, "%d;%s;%s;%s;%d;%d;%d;%d;%d;%d;%.3f;%d;%d",
                m->number, m->name, m->laboratory, m->lot.reference, m->lot.manufacturingDate.day,
                m->lot.manufacturingDate.month, m->lot.manufacturingDate.year, m->lot.expirationDate.day,
                m->lot.expirationDate.month, m->lot.expirationDate.year, m->price, m->quantityInStock,
                m->storageSector);

        fprintf(file, ";%d", m->supplierCount);
        for (j = 0; j < m->supplierCount; j++) {
            fprintf(file, ";%s;%s;%s", m->suppliers[j].name, m->suppliers[j].address,
                    m->suppliers[j].phone);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printf("Donnees sauvegardees dans %s.\n", DATA_FILE);
}

void loadData(Medication meds[], int *count) {
    FILE *file = fopen(DATA_FILE, "r");
    char line[MAX_LINE_LENGTH];
    int i;

    *count = 0;

    if (file == NULL) {
        return;
    }

    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return;
    }

    {
        char *endPtr;
        long parsedCount = strtol(line, &endPtr, 10);
        if (endPtr == line) {
            printf("Avertissement: format de fichier invalide.\n");
            fclose(file);
            return;
        }
        *count = (int)parsedCount;
    }
    if (*count < 0) {
        *count = 0;
    }
    if (*count > MAX_MEDICATIONS) {
        *count = MAX_MEDICATIONS;
    }

    for (i = 0; i < *count; i++) {
        char *token;
        int j;

        if (fgets(line, sizeof(line), file) == NULL) {
            printf("Avertissement: fichier de donnees corrompu a la ligne %d, %d medicament(s) charge(s).\n",
                   i + 2, i);
            *count = i;
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        token = strtok(line, ";");
        if (token == NULL) {
            *count = i;
            break;
        }
        meds[i].number = parseIntToken(token, 0);

        token = strtok(NULL, ";");
        safeStringCopy(meds[i].name, token, sizeof(meds[i].name));

        token = strtok(NULL, ";");
        safeStringCopy(meds[i].laboratory, token, sizeof(meds[i].laboratory));

        token = strtok(NULL, ";");
        safeStringCopy(meds[i].lot.reference, token, sizeof(meds[i].lot.reference));

        token = strtok(NULL, ";");
        meds[i].lot.manufacturingDate.day = parseIntToken(token, 0);

        token = strtok(NULL, ";");
        meds[i].lot.manufacturingDate.month = parseIntToken(token, 0);

        token = strtok(NULL, ";");
        meds[i].lot.manufacturingDate.year = parseIntToken(token, 0);

        token = strtok(NULL, ";");
        meds[i].lot.expirationDate.day = parseIntToken(token, 0);

        token = strtok(NULL, ";");
        meds[i].lot.expirationDate.month = parseIntToken(token, 0);

        token = strtok(NULL, ";");
        meds[i].lot.expirationDate.year = parseIntToken(token, 0);

        token = strtok(NULL, ";");
        meds[i].price = parseFloatToken(token, 0.0f);

        token = strtok(NULL, ";");
        meds[i].quantityInStock = parseIntToken(token, 0);

        token = strtok(NULL, ";");
        meds[i].storageSector = parseIntToken(token, 1);

        token = strtok(NULL, ";");
        meds[i].supplierCount = parseIntToken(token, 0);
        if (meds[i].supplierCount < 0) {
            meds[i].supplierCount = 0;
        }
        if (meds[i].supplierCount > MAX_SUPPLIERS) {
            meds[i].supplierCount = MAX_SUPPLIERS;
        }

        for (j = 0; j < meds[i].supplierCount; j++) {
            token = strtok(NULL, ";");
            safeStringCopy(meds[i].suppliers[j].name, token, sizeof(meds[i].suppliers[j].name));

            token = strtok(NULL, ";");
            safeStringCopy(meds[i].suppliers[j].address, token, sizeof(meds[i].suppliers[j].address));

            token = strtok(NULL, ";");
            safeStringCopy(meds[i].suppliers[j].phone, token, sizeof(meds[i].suppliers[j].phone));
        }
    }

    fclose(file);
}

void showMenu(void) {
    printf("\n===== Gestion Stock Pharmacie =====\n");
    printf("1. Ajouter un medicament\n");
    printf("2. Rechercher un medicament par numero\n");
    printf("3. Afficher tous les medicaments par secteur\n");
    printf("4. Afficher les medicaments par modele\n");
    printf("5. Afficher tous les fournisseurs\n");
    printf("6. Afficher les medicaments ordonnes par secteur/date\n");
    printf("7. Passer une commande\n");
    printf("8. Supprimer un medicament par reference lot\n");
    printf("9. Sauvegarder les donnees\n");
    printf("0. Quitter\n");
}

int main(void) {
    Medication medications[MAX_MEDICATIONS];
    int medicationCount = 0;
    int choice;

    loadData(medications, &medicationCount);
    printf("Donnees chargees: %d medicament(s).\n", medicationCount);

    while (1) {
        showMenu();
        choice = readInt("Choix: ");

        switch (choice) {
            case 1:
                addMedication(medications, &medicationCount);
                break;
            case 2:
                searchMedicationByNumber(medications, medicationCount);
                break;
            case 3:
                displayAllMedicationsBySector(medications, medicationCount);
                break;
            case 4:
                displayMedicationsByModel(medications, medicationCount);
                break;
            case 5:
                displayAllSuppliers(medications, medicationCount);
                break;
            case 6:
                displayAllMedicationsByOrderedDate(medications, medicationCount);
                break;
            case 7:
                placeOrder(medications, medicationCount);
                break;
            case 8:
                deleteMedicationByBatchReference(medications, &medicationCount);
                break;
            case 9:
                saveData(medications, medicationCount);
                break;
            case 0: {
                char saveChoice[10];
                readLine("Sauvegarder avant de quitter ? (o/n): ", saveChoice, sizeof(saveChoice));
                if (tolower((unsigned char)saveChoice[0]) == 'o') {
                    saveData(medications, medicationCount);
                }
                printf("Au revoir.\n");
                return 0;
            }
            default:
                printf("Choix invalide.\n");
        }
    }
}
