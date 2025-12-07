#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void addField(){
    int n;
    printf("Warning: This will overwrite your existing tracked fields!\n");
    printf("Are you sure you want to continue? (y/n): ");
    char confirm;
    scanf(" %c", &confirm);
    getchar();  // consume newline
    if (confirm != 'y' && confirm != 'Y') {
        printf("Action cancelled. No changes made.\n");
        return;
    }
    printf("How many fields you want to track?\n");
    scanf("%d",&n);
    getchar();          //consume newline after scanf()
    FILE *temp = fopen("temp.txt","w");
    if(!temp){
        printf("FILE temp couldn't be opened.\n");
        return;
    }
    fprintf(temp, "FIELDS: ");    //writing header
    for(int i=0;i<n;i++){
        printf("Enter name of field %d: ",i+1);
        char *field = NULL;
        int ch, len=0, size=1;
        field = malloc(size);
        if(!field){
            printf("Memory allocation failed.\n");
            fclose(temp);
            return;
        }
        while ((ch = getchar()) != '\n' && ch != EOF) {
            field = realloc(field, ++size);     // 1 for new char and 1 for '\0'
            field[len++] = ch;
        }
        field[len] = '\0';
        fprintf(temp, "%s", field);
        if (i != n - 1) {
            fprintf(temp, ", ");
        } else {
            fprintf(temp, "\n");
        }
        free(field);
    }

    FILE *old = fopen("track.txt", "r");
    if(old != NULL){
        char *line = NULL;
        size_t len=0;
        int skip = 1;
        while(getline(&line, &len, old) != -1){
            if (skip) {
                skip = 0;
                continue; // Skip old FIELDS line
            }
            fputs(line, temp);
        }
        free(line);
        fclose(old);
    }
    fclose(temp);
    if (remove("track.txt") != 0) {
        printf("Error deleting original track.txt\n");
    } else if (rename("temp.txt", "track.txt") != 0) {
        printf("Error renaming temp.txt to track.txt\n");
    } else {
        printf("Fields saved successfully.\n");
    }
}

void addLog(){
    FILE *fp = NULL;
    fp = fopen("track.txt","a+");
    if(fp==NULL){
        addField();     // file does not exist call addField()
    }
    else{
        int ch = fgetc(fp);
        if(ch == EOF){
            addField();      // file is empty so initialize again
        }
        rewind(fp);       // reset pointer before closing
        fclose(fp);
    }
    // Reopen to read the FIELDS
    fp = fopen("track.txt","a+");
    if (fp == NULL) {
        printf("Something went wrong.\n");
        return;
    }
    rewind(fp); // Move to start of file to read header
    char *line = NULL;
    size_t len =0;      // to study
    getline(&line, &len, fp);     // Read FIELDS
    if(line == NULL){
        printf("Header missing...\n");
        fclose(fp);
        return;
    }
    // tokenize the FIELDS
    char *token = strtok(line+8,","); //to study    // skip "FIELDS: "
    char **fields = NULL;
    int fieldCount = 0;
    while(token != NULL){
        while(*token == ' ') token++;    //skip leading spaces
        // Remove trailing spaces and newline
        int len = strlen(token);
        while(len > 0 && (token[len-1] == ' ' || token[len-1] == '\n')){
            token[--len] = '\0';
        }
        fields = realloc(fields, (fieldCount + 1) * sizeof(char *));
        fields[fieldCount] = malloc(len + 1);
        strcpy(fields[fieldCount], token);
        fieldCount++;
        token = strtok(NULL, ",");    //to study
    }
    // Ask user for date
    char date[14];
    printf("Enter Date (DD-MM-YYYY): ");
    scanf("%s", date);
    getchar();      //consume newline
    // open file to append log
    FILE *out = fopen("track.txt", "a");
    if (out == NULL) {
        printf("Failed to open file for writing log.\n");
        return;
    }
    fprintf(out, "------------------------\n");
    fprintf(out, "Date       : %s\n", date);
    for (int i = 0; i < fieldCount; i++) {
        char input[100];
        printf("Enter %s: ", fields[i]);
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0'; // remove newline
        fprintf(out, "%-11s: %s\n", fields[i], input);
        free(fields[i]);
    }
    fprintf(out, "------------------------\n");
    free(fields);
    free(line);
    fclose(out);

    printf("Log saved successfully!\n");
}

void analysis() {
    const char *python_command = ".\\\\.venv\\\\Scripts\\\\python.exe -u analysis.py";
    printf("Executing command:\n%s\n\n", python_command);
    int result = system(python_command);
}

void viewLog(){
    char searchDate[14];
    printf("Enter date to see log (DD-MM-YYYY):");
    scanf("%s",searchDate);
    getchar();  // consume newline if needed
    FILE *fp = fopen("track.txt","r");
    if(fp == NULL){
        printf("No log file found.\n");
        return;
    }
    char *line = NULL;
    size_t len = 0;
    int found = 0;
    int printing = 0;
    while(getline(&line, &len, fp) != -1){
        //check for the start of a log block
        if(strncmp(line, "Date     : ",11) == 0){    //  Because "Date : " is exactly 11 characters long. Hum sirf itna hi compare karna chahte hain, date ko nahi.
            // If already found one log, and new "Date" comes, break
            if (found) break;
            char actualDate[20];
            sscanf(line, "Date     : %s", actualDate);
            //check if this is the desired date
            if(strcmp(actualDate, searchDate) == 0){
                found = 1;
                printing = 1;
                printf("\n--- Log Found ---\n");
                printf("%s", line);  // print date line
            }
            else{
                printing = 0;
            }
        }
        else if(printing){
            // print all lines untill next "Date     : " appears
            printf("%s",line);
        }
    }
    if(!found){
        printf("No log found for date %s.\n", searchDate);
    }
    free(line);
    fclose(fp);

    // strncmp starting line check karta hai jaise kisi diary ke new page ki shuruaat.
}

int main() {
    int choice;
    do{
        printf("Health Tracker Menu:\n");
        printf("1. Add new log\n");
        printf("2. View existing log\n");
        printf("3. Add new Field\n");
        printf("4. Data Analysis (Python script)\n");
        printf("5. Exit the program\n");
        scanf("%d",&choice);
        switch (choice) {
            case 1:
                addLog();
                break;
            case 2:
                viewLog();               
                break;
            case 3:
                addField();
                break;
            case 4:
                analysis();
                printf("Running data analysis...\n");
            case 5:
                printf("Thank you for using the Tracker...\n");
                break;
            default:
                printf("Invalid choice! Please select 1, 2, 3 or 4.\n");
            }
    }while(choice!=5);
    return 0;
}