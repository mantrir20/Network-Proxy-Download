/* f20171340@hyderabad.bits-pilani.ac.in Raunak Mantri */

/*The program is used to download html page and its image using sockets in c. The Steps involved are-
1) Command line arguments are stored in respective character array variables.
2) A TCP socket is created socket function. The parametrs used are AF_INET, SOCK_STREAM (for TCP).

----FOR HTML---
3) Then a request is generated by concatenation of dynamic command line arguments given.
4) The request is sent using the socket created.
5) While recieving the request, a file pointer is created and the file is opened using w+.
6) The response is recorded in a string buffer, parsed to get the content length and then is made to loop the requested response until the ( content length + header response length ) is less or equal to bytes read by recv function.
7) For this purpose, the bytes read by the recv function is recorded at each call. (Since that is variable is each iteration and also dynamic)
8) Now, to handle the HTTP 30X redirects, a flag variable called header_length_flag is used. If this becomes true, first the response is parsed till Location: and then the Location of redirected url is stored and the existing socket is closed.
9) Then again a new socket is created and a new request is sent with the updated url.
10) Since there is a possibility of multiple redirects, a recusrrsion call is made using user defined create_socket function.
11) Now, since the html should be header free, first the response is parsed till the end of header and only the response is written into the file. Corresponding flags are used for this purpose.
12) Also, a flag variable called check_status is defined so that only HTTP 200 and 30X are dealt with the program.

----FOR IMAGE---
1) A similar procedure is followed for the image.
2) However, here the fileopen is given wb+ permission.
3) Similar flags like the html one is used for checking the edge cases here.

------------------------End of program desciption*/
//Start of the program

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>

#define SIZE 1000 

char char_set[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; 
char* encodePass(char input_str[]) ;
int shift_val(int value);
int temp;

// ---  Flags for checking the edge cases and redirects and content length
int msg_length = 0;
int msg_length_flag = 0;
int header_length = 0;
int header_length_flag = 0;
int run = 0;
int image_flag = 0;
int check_status =0;
int redirect_flag = 0;

int padding = 0;


char url[100], ip_address[100], port_no[100], username[100], passw[100], output_filename[100], logo_name[100], redirected_url[100] = "";
char string_decode[1000];

void create_socket()
{
    //printf("REDIRECTING\n");
    // reseting the flags to 0
    msg_length = 0;
    msg_length_flag = 0;
    header_length = 0;
    header_length_flag = 0;
    run = 0;
    image_flag = 0;
    check_status =0;

    char value[1000] = "";
    strcat(value, username);
    strcat(value, ":");
    strcat(value, passw);
    int len_str = strlen(value);
    char string_decode[1000];
    strcpy(string_decode, encodePass(value));

    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(port_no));
    inet_aton(ip_address, &server_address.sin_addr);
    // -- Done --//
    
    //-- GET HTML DOCUMENT --//
    connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    char response[1024];
    char request1[1000] = "GET ";
    //strcat(request1, "http://");
    strcat(request1, redirected_url);
    strcat(request1, " HTTP/1.1\r\nProxy-Authorization: Basic ");
    strcat(request1, string_decode);
    strcat(request1, "\r\n\r\n");

    //printf("%s", request1);
    send(client_socket, request1, sizeof(request1), 0);
    FILE *fp3;
    
    fp3 = fopen(output_filename, "w+");
    if(fp3 == NULL)
    {
        printf("File cannot be opened");
        return;
    }
    
    int bytes_read;
    int curr_length=0;
    do {
        
        bytes_read = recv(client_socket, &response, sizeof(response), 0);
        
        if(header_length_flag == 0)
        {
            char *pointer_redirect = strstr(response, "HTTP/1.1");
            pointer_redirect = pointer_redirect + 9;
            if(pointer_redirect != NULL && *pointer_redirect == '3')
            {
                printf("HEY");
                pointer_redirect = strstr(response, "Location: ");
                pointer_redirect = pointer_redirect + 10;
                int i = 0;
                while(*pointer_redirect != '\r')
                {
                    redirected_url[i] = *pointer_redirect;
                    pointer_redirect++;
                    i++;
                }
                redirected_url[i] = '\0';
                fclose(fp3);
                close(client_socket);
                create_socket();
                return;
            }
            if(pointer_redirect != NULL && *pointer_redirect != '2') check_status = 1;
        }
            
        if(header_length_flag == 0)
        {
            char *pointer_head_end = strstr(response, "keep-alive");
            if(pointer_head_end != NULL)
            {
                char *start = &response[0];
                while(start != pointer_head_end) 
                {
                    header_length++;
                    start++;
                }
                header_length_flag++;
                header_length += 14;
            }
        }

        if(msg_length_flag == 0)
        {
            char *pointer_msg_len = strstr(response, "Content-Length: ");
            if(pointer_msg_len != NULL)
            {
                pointer_msg_len = pointer_msg_len + 16;
                while(*pointer_msg_len != '\r')
                {
                    msg_length = msg_length*10 + (*pointer_msg_len)-48;
                    pointer_msg_len++;
                }
                msg_length_flag++;
            }
        }

        if (bytes_read == -1) {
            printf("recv");
        }
        else {
            if(run == 0)
            {
                fwrite(response + header_length, sizeof(char), bytes_read - header_length,fp3);
                run++;
                printf("Header Length = %d\n", header_length);
                printf("Messgae Length = %d\n", msg_length);
            }
            else fwrite(response, sizeof(char), bytes_read,fp3);
        }
        
        curr_length += bytes_read;
    } while (curr_length < header_length + msg_length && check_status == 0);

    fclose(fp3);
    close(client_socket);
    
    //---END--//

    char check_url[100] = "info.in2p3.fr";
    if(strcmp(url,check_url) == 0)
    {
        int client_socket_image;
        client_socket_image = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_address_image;
        server_address_image.sin_family = AF_INET;
        server_address_image.sin_port = htons(atoi(port_no));
        inet_aton(ip_address, &server_address_image.sin_addr);
        connect(client_socket_image, (struct sockaddr *) &server_address_image, sizeof(server_address_image));
        
        msg_length = 0;
        msg_length_flag = 0;
        header_length = 0;
        header_length_flag = 0;
        run = 0;

        char response1[2048];
        char request2[100] = "GET http://info.in2p3.fr/";
        strcat(request2, "cc.gif ");
        strcat(request2, "HTTP/1.1\r\nProxy-Authorization: Basic ");
        
        strcat(request2, string_decode);
        strcat(request2, "\r\n\r\n");
            
        send(client_socket_image, request2, sizeof(request2), 0);
        
        FILE *fp2;
        fp2 = fopen(logo_name, "wb+");
        if(fp2 == NULL)
        {
            printf("File cannot be opened");
            exit(1);
        }
        
        int bytes_read = 0;
        int curr_length = 0;
        do {
            bytes_read = recv(client_socket_image, &response1, sizeof(response1), 0);
            
            if(header_length_flag == 0)
            {
                char *pointer_head_end = strstr(response1, "keep-alive");
                char *start = &response1[0];
                while(start != pointer_head_end) 
                {
                    header_length++;
                    start++;
                }
                header_length_flag++;
                header_length += 14;
            }

            if(msg_length_flag == 0)
            {
                char *pointer_msg_len = strstr(response1, "Content-Length: ");
                pointer_msg_len = pointer_msg_len + 16;
                while(*pointer_msg_len != '\r')
                {
                    msg_length = msg_length*10 + (*pointer_msg_len)-48;
                    pointer_msg_len++;
                }
                msg_length_flag++;
            }

            if (bytes_read == -1) {
                printf("recv");
            }
            else {
                if(run == 0)
                {
                    fwrite(response1 + header_length, sizeof(char), bytes_read - header_length,fp2);
                    run++;
                    printf("Header Length = %d\n", header_length);
                    printf("Messgae Length = %d\n", msg_length);
                }
                else fwrite(response1, sizeof(char), bytes_read,fp2);
            }
            curr_length += bytes_read;
        } while (curr_length < header_length + msg_length);

        fclose(fp2);
        close(client_socket_image);
    }
    
}

int main(int argc, char *argv[])
{
    
    strcpy(url,argv[1]);
    strcpy(ip_address, argv[2]);
    strcpy(port_no, argv[3]);
    strcpy(username, argv[4]);
    strcpy(passw, argv[5]);
    strcpy(output_filename, argv[6]);
    strcpy(logo_name, argv[7]);

    //-- Decoding the username and password --//
    char value[1000] = "";
    strcat(value, username);
    strcat(value, ":");
    strcat(value, passw);
    int len_str = strlen(value);
    char string_decode[1000];
    strcpy(string_decode, encodePass(value));
    //-- Decode Complete --//

    //-- Creating a socket For HTML FILE--//
    int client_socket;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(port_no));
    inet_aton(ip_address, &server_address.sin_addr);
    // -- Done --//
    

    //-- GET HTML DOCUMENT --//
    connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));
    
    
    char response[1024];
    char request1[1000] = "GET ";
    strcat(request1, "http://");
    strcat(request1, url);
    strcat(request1, " HTTP/1.1\r\nProxy-Authorization: Basic ");
    strcat(request1, string_decode);
    strcat(request1, "\r\n\r\n");

    //printf("%s", request1);
    send(client_socket, request1, sizeof(request1), 0);
    
    FILE *fp;
    fp = fopen(output_filename, "w+");
    if(fp == NULL)
    {
        printf("File cannot be opened");
        exit(1);
    }
    
    int bytes_read;
    int curr_length=0;
    do {
        
        bytes_read = recv(client_socket, &response, sizeof(response), 0);

        if(header_length_flag == 0)
        {
            char *pointer_redirect = strstr(response, "HTTP/1.1");
            pointer_redirect = pointer_redirect + 9;
            if(pointer_redirect != NULL && *pointer_redirect == '3')
            {
                pointer_redirect = strstr(response, "Location: ");
                pointer_redirect = pointer_redirect + 10;
                int i = 0;
                while(*pointer_redirect != '\r')
                {
                    redirected_url[i] = *pointer_redirect;
                    pointer_redirect++;
                    i++;
                }
                //printf("%s", redirected_url);
                redirected_url[i] = '\0';
                fclose(fp);
                close(client_socket);
                create_socket();
                return 0;
            }
            if(pointer_redirect != NULL && *pointer_redirect != '2') check_status = 1;
        }
            
        if(header_length_flag == 0)
        {
            char *pointer_head_end = strstr(response, "keep-alive");
            if(pointer_head_end != NULL)
            {
                char *start = &response[0];
                while(start != pointer_head_end) 
                {
                    header_length++;
                    start++;
                }
                header_length_flag++;
                header_length += 14;
            }
        }

        if(msg_length_flag == 0)
        {
            char *pointer_msg_len = strstr(response, "Content-Length: ");
            if(pointer_msg_len != NULL)
            {
                pointer_msg_len = pointer_msg_len + 16;
                while(*pointer_msg_len != '\r')
                {
                    msg_length = msg_length*10 + (*pointer_msg_len)-48;
                    pointer_msg_len++;
                }
                msg_length_flag++;
            }
        }

        if (bytes_read == -1) {
            printf("recv");
        }
        else {
            if(run == 0)
            {
                fwrite(response + header_length, sizeof(char), bytes_read - header_length,fp);
                run++;
                //printf("Header Length = %d\n", header_length);
                //printf("Messgae Length = %d\n", msg_length);
            }
            else fwrite(response, sizeof(char), bytes_read,fp);
        }
        curr_length += bytes_read;
    } while (curr_length < header_length + msg_length && check_status == 0);

    fclose(fp);
    close(client_socket);
    
    //---END--//

    char check_url[100] = "info.in2p3.fr";
    if(strcmp(url,check_url) == 0)
    {
        int client_socket_image;
        client_socket_image = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_address_image;
        server_address_image.sin_family = AF_INET;
        server_address_image.sin_port = htons(atoi(port_no));
        inet_aton(ip_address, &server_address_image.sin_addr);
        connect(client_socket_image, (struct sockaddr *) &server_address_image, sizeof(server_address_image));
        
        msg_length = 0;
        msg_length_flag = 0;
        header_length = 0;
        header_length_flag = 0;
        run = 0;

        char response1[2048];
        char request2[100] = "GET http://info.in2p3.fr/";
        strcat(request2, "cc.gif ");
        strcat(request2, "HTTP/1.1\r\nProxy-Authorization: Basic ");
        
        strcat(request2, string_decode);
        strcat(request2, "\r\n\r\n");
            
        send(client_socket_image, request2, sizeof(request2), 0);
        
        FILE *fp2;
        fp2 = fopen(logo_name, "wb+");
        if(fp2 == NULL)
        {
            printf("File cannot be opened");
            exit(1);
        }
        
        int bytes_read = 0;
        int curr_length = 0;
        do {
            bytes_read = recv(client_socket_image, &response1, sizeof(response1), 0);
            
            if(header_length_flag == 0)
            {
                char *pointer_head_end = strstr(response1, "keep-alive");
                char *start = &response1[0];
                while(start != pointer_head_end) 
                {
                    header_length++;
                    start++;
                }
                header_length_flag++;
                header_length += 14;
            }

            if(msg_length_flag == 0)
            {
                char *pointer_msg_len = strstr(response1, "Content-Length: ");
                pointer_msg_len = pointer_msg_len + 16;
                while(*pointer_msg_len != '\r')
                {
                    msg_length = msg_length*10 + (*pointer_msg_len)-48;
                    pointer_msg_len++;
                }
                msg_length_flag++;
            }

            if (bytes_read == -1) {
                printf("recv");
            }
            else {
                if(run == 0)
                {
                    fwrite(response1 + header_length, sizeof(char), bytes_read - header_length,fp2);
                    run++;
                    //printf("Header Length = %d\n", header_length);
                    //printf("Messgae Length = %d\n", msg_length);
                }
                else fwrite(response1, sizeof(char), bytes_read,fp2);
            }
            curr_length += bytes_read;
        } while (curr_length < header_length + msg_length);

        fclose(fp2);
        close(client_socket_image);
    }
    
    
    return 0;

}

int shift_val(int value)
{
	value = value << 8;
	return value;
}

char* encodePass(char input_str[]) 
{ 
    int len_str = strlen(input_str);
    char *string_res = (char *) malloc(SIZE * sizeof(char)); 
      
    int index, nfbits = 0, val = 0;
    int count = 0;
    int i=0, j;
    int k = 0; 
    
    int cal = shift_val(50);
    cal++;
      
    	i = 0;
    	while(i < len_str)
        { 
            val = 0, count = 0, nfbits = 0; 
  
            for (j = i; j < len_str && j <= i + 2; j++) 
            {  
                val = shift_val(val);  
                val = val | input_str[j];  
                count = count + 1;
                cal = shift_val(50);
                cal++;
            } 
  
            nfbits = count * 8;  
            padding = nfbits % 3;  
  
            while (nfbits != 0)  
            { 
                if (nfbits >= 6) 
                { 
                    temp = nfbits - 6; 
                    cal++;
                    index = (val >> temp) & 63;  
                    nfbits -= 6;   
                    cal = shift_val(50);  
                    cal++;     
                } 
                else
                { 
                    temp = 6 - nfbits; 
                    cal++;
                    index = (val << temp) & 63;  
                    nfbits = 0; 
                    cal = shift_val(50);
                    cal++;
                } 
                string_res[k++] = char_set[index]; 
            } 
            i = i + 3;
    } 
  
    i = 1;
    while(i <= padding) 
    { 
        string_res[k++] = '='; 
        i++;
        cal++;
    } 
  
    string_res[k] = '\0'; 
  
    return string_res; 
} 



