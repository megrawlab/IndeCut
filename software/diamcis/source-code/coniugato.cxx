#include<iostream>
using namespace std;

void coniugato(int nr, int nc, int c[],int cc[],int cc2[],int K[],int
v[],int& nnodi,int r[]){

int i,j,k,sommar,sommacc2;

for(i=0;i<nc;i++)

//building coniugator at the first step

for (j=0;j<nr;j++) {
   k=0; 
   for (i=0;i<nc;i++){
     if (c[i]>=j+1) k=k+1;
   }
   cc[j]=k;
}


//building the coniugator at the second step  
   

for (j=0;j<nr;j++){
  if ((j+1)>c[0]) cc2[j]=cc[j];
   else cc2[j]=cc[j]-1;
  //cout<<"cc2 "<<cc2[j]<<endl; 
}


//praparing constraints vectors: positions and number of 1s


j=0;

for(k=0;k<nr;k++){
sommar=0; 
sommacc2=0;
  for (i=0;i<=k;i++){
     sommar=sommar+r[i];
     sommacc2=sommacc2+cc2[i];
  }
  if (sommar>sommacc2) {
    K[j]=k;
    v[j]=sommar-sommacc2;
    j=j+1;
  } 
}
  
nnodi=j;

//deleting redondant nodes

for(i=0;i<nnodi-1;i++){
  for(j=i+1;j<nnodi;j++){
    if (v[j]<=v[i]){
      for(k=j+1;k<nnodi;k++){ 
        K[k-1]=K[k];
	v[k-1]=v[k];
      }
      j=j-1;
      nnodi=nnodi-1;
    }
  }
} 
   
 
for (j=1;j<nnodi;j++){
  for(i=0;i<j;i++){
   if(v[j]-v[i]>=K[j]-K[i]){
    for(k=i+1;k<nnodi;k++){
        K[k-1]=K[k];
	v[k-1]=v[k];
      }
      i=i-1;
      j=j-1;
      nnodi=nnodi-1;
    }
  }
}           
 
     
}
