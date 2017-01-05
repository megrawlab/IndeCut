#include<iostream>
#include<fstream>
#include"coniug.h"
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
using namespace std;



/* legend of inserted arguments:

argv[0] = program name
argv[1] = input file where the network is saved
argv[2] = output file where randomized networks will be saved
argv[3] = output file where the probabilities of the networks will be saved
argv[4] = sample size




*/

int rand(void);
void srand(unsigned int seed);

int main(int argc,char *argv[]){

srand(time(NULL));

int i;

cout<<"number of parameters "<<argc<<endl;

 if(argc<5) 
   {
     printf("to few parameters\n");
     exit(1);
   } 

for(i=0;i<argc;i++){
 cout<<"parameter "<<i+1<<" "<<argv[i]<<endl;
} 




int nmatrice;

nmatrice=atoi(argv[4]);



int n,gene;

ifstream z(argv[1]);
ofstream out1(argv[2]);
ofstream out2(argv[3]); 
 //reading the network the program understands how big the matrix is
 
z>>n; 
 
while(1>0){
 z>>gene;
  if(!z.eof()){
   if(n<gene) n=gene;
  } else break;
}  

  
z.close();
  
//n is the dimension of the matrix

bool matrix[n][n];
int j;

for(i=0;i<n;i++){
 for(j=0;j<n;j++)
  matrix[i][j]=false;
}  

//filling initial matrix
ifstream zi(argv[1]);

while(1>0){
 zi>>i;
 if(!zi.eof()){
  zi>>j;
  matrix[i-1][j-1]=true;
 } else break;
}  

zi.close(); 
 

int rvero[n],cvero[n];

//finding row and column sums


for(j=0;j<n;j++){
 cvero[j]=0;
 for(i=0;i<n;i++){
  if(matrix[i][j]==true) cvero[j]=cvero[j]+1;
 }
}

for(i=0;i<n;i++){
 rvero[i]=0;
 for(j=0;j<n;j++){
  if(matrix[i][j]==true) rvero[i]=rvero[i]+1;
 }
}
   
 


int nr,nc;
nr=n;
nc=n;



int k,ti;
bool A[nr][nc];
double prob,x;


//------------------------cycle on the matrices

for(ti=0;ti<nmatrice;ti++){

 
 cout<<"I am generating matrix "<<ti<<endl;
 
//initializing matrix and probability variables  

for(i=0;i<nr;i++){
  for(j=0;j<nc;j++)
    A[i][j]=false;
}  
    
prob=0.;

//reading row and column sums


int pos[nr],pos2[nr],r[nr],c[nc];


for (i=0;i<nr;i++){
   r[i]=rvero[i];
   pos[i]=nr-1-i;
}


 

ordina(r,pos,nr);
  
   
for (i=0;i<nc;i++){
   c[i]=cvero[i];
   
}  

int ncn,t;
ncn=nc;


//------------------------------------------------cycle on the columns

for(t=0;t<nc;t++){


int cc[nr],k,cc2[nr],K[nr],v[nr],nnodi,cagg[ncn];

//building coniugation of column sums and passage of constraints


nnodi=0;
for(i=0;i<nr;i++){
 cc[i]=0;
 cc2[i]=0;
 K[i]=0;
 v[i]=0;
} 

for(i=0;i<ncn;i++)
  cagg[i]=c[i];

coniugato(nr,ncn,cagg,cc,cc2,K,v,nnodi,r);


//finding how many 1 have to be put 

int veff[nnodi];


//preparing the map for extraction of positions

bool a[nr];


for(i=0;i<nr;i++)
 a[i]=false;
 
for(i=0;i<nnodi;i++)
 veff[i]=0; 
 
mappatura3(v,cagg[0],nr,r,K,veff,a,nnodi,prob,ncn);



for(i=0;i<nr;i++)
pos2[i]=pos[i];

ordinabool(pos2,a,nr);

//filling column t

for(i=0;i<nr;i++){
  A[i][t]=a[i];
} 

//reorganizing after the filling for next column

ordina(pos,r,nr); 


  
for(i=0;i<nr;i++){
 if(A[i][t]==true) 
   r[i]=r[i]-1; 
} 

ordina(r,pos,nr);

  

for(i=1;i<ncn;i++){
  c[i-1]=c[i]; 
}


ncn=ncn-1;

}

//--------------------------------------------end cycle on the column


//writing randomized network and probability

out2<<prob<<endl; 

for(j=0;j<nr;j++){
 for(k=0;k<nc;k++){
 if(A[j][k]==true) out1<<j+1<<"  "<<k+1<<endl;
 }
} 

out1<<endl;



}

//----------------------------------------------end cycle on the matrix

out1.close();
out2.close();
   

return 0;
}  
