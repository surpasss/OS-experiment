#include<stdio.h>    
#include<math.h>    
int main()    
{    
    int hang,lie;    
    int n=10;
    char zi='A';    
    char a[100][100];          
    for(hang=1;hang<=n;hang++)    
    {    
        for(lie=1;lie<=2*n-1;lie++)    
        {    
            a[hang][lie]=zi+((hang-1)%26);    
            if(a[hang][lie]>'Z')    
            	a[hang][lie]-=26;    
        }    
    }    
    for(lie=1;lie<=2*n-1;lie++)    
    {    
        a[n][lie]=a[n][n]+(abs(n-lie)%26);    
        if(a[n][lie]>'Z')    
        a[n][lie]-=26;    
    }    
    for(hang=2;hang<=n-1;hang++)    
    {    
        a[hang][1]=a[n][1]+(n-hang)%26;    
        if(a[hang][1]>'Z')    
        	a[hang][1]-=26;      
        a[hang][2*n-1]=a[n][1]+(n-hang)%26;    
        if(a[hang][2*n-1]>'Z')    
       		a[hang][2*n-1]-=26;    
    }   
    for(hang=1;hang<=n;hang++)    
    {    
        for(lie=1;lie<=2*n-2;lie++)    
        {    
            if(lie==1||hang==n||hang==lie||hang+lie==2*n)              
				printf("%c ",a[hang][lie]);    
            else printf("  ");    
        }    
        printf("%c\n",a[hang][2*n-1]);    
    }    
    printf("\n"); 
}  
