/* { dg-do compile } */

typedef struct {
       int     d1;                                                            
       int     d2;                                                            
} DT;                                                                           

typedef struct {
       int     t1;                                                            
       DT      t2[2];                                                         
} TBL;                                                                          

TBL     tbl;                                                                    
DT      dt;                                                                     

void
foo( int x )
{
       dt = tbl.t2[x-1];                                                      
}
