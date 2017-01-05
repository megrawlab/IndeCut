#include<iostream>
using namespace std;

double Rbis(int n, bool C[],int nS,double w[]){
int i,j,k,c,m;
double r[n+1][nS+1];
c=0;
m=nS;
for(i=0;i<nS;i++){
 if(C[i]==false){
  m=m-1;
  for(j=i-c;j<m;j++){
   w[j]=w[j+1]; 
  }
  c=c+1;
 }
}   
for(i=0;i<=m;i++){
 r[0][i]=1.;
 for(k=1;k<=n;k++)
  r[k][i]=0.;
} 
for(k=1;k<=n;k++){
 for(i=k;i<=m;i++){
  r[k][i]=r[k][i-1]+r[k-1][i-1]*w[i-1]; 
 }
} 

return r[n][m];
} 
