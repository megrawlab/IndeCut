#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<iostream>
#include<fstream>
#include<math.h>

using namespace std;

int rand(void);


double Rbis(int n, bool C[],int nS,double w[]);


void mappatura3(int v[],int b,int nr,int r[],int K[],int veff[],bool
a[],int nnodi,double& prob,int ncn){


int z,t,o,l;
double y,ausilio,ausiliobis,ausiliotris;

//------------------------------------------------------cycle on the knots

for(t=0;t<nnodi;t++){



//preparing maps for extraction positions

int sommar,i,j,k,c;

//defining variables for probability calculation
int nS;

if(t==0) c=0;
 else c=K[t-1]+1;

//lenth of wight vector 
nS=K[t]-c+1;

//preparing ensamble vector
bool C[nS];
double w[nS];
int unofissato;
unofissato=0;
for(i=0;i<nS;i++){
 if(ncn==r[i+c]){
  C[i]=false;
  unofissato=unofissato+1;
 } 
 else { 
  C[i]=true;
  w[i]=r[i+c]*1./(ncn-r[i+c]);
 } 

} 


sommar=r[c];

int mappa[K[t]+1-c][2];

mappa[0][0]=0;
mappa[0][1]=r[c];


for(i=1;i<K[t]+1-c;i++){
  mappa[i][0]=mappa[i-1][1];
  mappa[i][1]=mappa[i][0]+r[c+i];
  sommar=sommar+r[c+i];

} 

//after every extraction the map must be rearrenged

//extracting how many 1s for every constraint

double x;

if (c==0){
 x=( (double)rand()/((double)(RAND_MAX)+(double(1))));
 x=v[0]+x*abs(min(K[0]+1,b)-v[0]);
 x=x+0.5;
 veff[0]=int(x); 
 prob=prob-log10(min(K[0]+1,b)-v[0]+1);
 b=b-veff[0];
 o=veff[0];
}
else{
  x=( (double)rand()/((double)(RAND_MAX)+(double(1))));
  x=max(v[t]-o,0)+x*abs(max(v[t]-o,0)-min(K[t]-K[t-1],b));
  x=x+0.5;
  veff[t]=int(x);
  prob=prob-log10(min(K[t]-K[t-1],b)-max(v[t]-o,0)+1);
  b=b-veff[t];
  o=o+veff[t];
}

//extracting positions

for(i=0;i<veff[t];i++){
  x=( (double)rand()/((double)(RAND_MAX)+(double(1))));
  x=x*sommar;
  for(j=0;j<K[t]+1-c;j++){
    if((x>=mappa[j][0])&&(x<=mappa[j][1])) 
    break;
  }
  a[c+j]=true;
  
  for(k=j;k<K[t]+1-c;k++){
    mappa[k][1]=mappa[k][1]-r[c+j];
    mappa[k+1][0]=mappa[k][1];
  }
  sommar=sommar-r[c+j];  
}

//final calculation of the probability of the 0-1 sequence

if(veff[t]!=nS){
 for(i=0;i<nS;i++){
  if(a[i+c]==true){
   prob=prob+log10(w[i]);
  }
 }
 prob=prob-log10(Rbis(veff[t],C,nS,w));
}   

 
}   


} 
