#include <stdio.h>
#include <stdlib.h>
#include <string.h>
	
void INITIALIZE (int N, unsigned char **arenaChar) {	
	*arenaChar = (unsigned char*)calloc(N, sizeof(unsigned char));/*va aloca N bytes
																	din memorie si-i va initializa cu 0*/
	if (arenaChar == NULL) {										
		exit(-1);
	}
}

void FINALIZE (unsigned char *arenaChar) {
	free(arenaChar);//va elibera zona de memorie ocupata
	exit(-1);//va finaliza programul
}

void DUMP (unsigned char *arenaChar, int N) {
	int i;
	for (i = 0; i < N; i++) {
		if (i % 16 == 0) {
			if (i != 0) {
				printf("\n");/*va trece la o linie noua daca s-a ajuns la un octet de pe o
								 pozitie care este multiplu de 16*/
			}
			printf("%08X\t", i);//afisarea indicilor
		}
	
	if ((i + 1) % 8 == 0 && (i + 1) % 16 != 0) {//daca s-a ajuns la al 8-lea byte pe o linie
			printf("%02X  ", arenaChar[i]);//va afisa un spatiu suplimentar
		} else {
			printf("%02X ", arenaChar[i]);
		}
	}
	
	printf("\n");
	printf("%08X\n", N);//afisarea indexului ultimului octet din arena + 1
}

int ALLOC (unsigned char *arenaChar, int size, int N) {
	int *arenaInt = (int*)arenaChar;/*poniterul arenaChar va retine mereu adresa
									 primului byte din arena, iar arenaInt va fi folosit pentru conversii la int
									 pentru modificarea sectiunii de gestiune*/
	if (*arenaInt == 0) { //daca nu a mai avut loc o alocare de memorie
		if (N - 1 - 4 >= size + 3 * sizeof(int)) {//daca exista suficent spatiu pentru alocare
			arenaInt[0] = 4;//modific indexul de start
			arenaInt[1] = 0;/*modific sectiunea de gestiune a blocului alocat,
							 care va fi in continuarea indexului de start*/
			arenaInt[2] = 0;
			arenaInt[3] = size + 3 * sizeof(int);
			arenaInt += 4;
			printf("%d\n", (int)((unsigned char*)arenaInt - arenaChar));/*afisarea indexului 
																		 sectiunii de date*/
			return (unsigned char*)arenaInt - arenaChar;
		} else{
			printf("%d\n", 0);//afisez 0 daca nu exista spatiu pentru alocare
			return 0;
		}
	} else { 
		unsigned char *arenaChar0 = arenaChar,/*arenaChar0 va retine adresa inceputului 
											sectiunii de gestiune a blocului care precede zona de memorie libera analizata*/
						*arenaChar1 = arenaChar,//arenaChar1 va retine adresa inceputului zonei de memorie libera
						*arenaChar2 = arenaChar;/*arenaChar2 va retine adresa sfaristului zonei de memorie libera
									 			va avea aceeasi valoare ca arenaChar1 in cazul in care nu s-a gasit
									 			 o zona de momerie libera*/
		int alocareEfectuata = 0; //presupunem ca alocarea nu a avut loc

		arenaChar1 += sizeof(int);//trece la sfarsitul indexului de start
		arenaChar2 += arenaInt[0];/*trece la inceputul primului bloc de memorie dupa
							indexul de start (echivalent cu sfarsitul zonei libere de memorie, daca exista)*/
		
		do {
			if (arenaChar2 - arenaChar1 == 0) {//daca nu s-a gasit o zona libera de memorie
				arenaChar0 = arenaChar1;//se continua cautarea
				arenaInt = (int*)arenaChar1;
				arenaChar1 = arenaChar2 + arenaInt[2];
				if (arenaInt[0] != 0) {
					arenaChar2 = arenaChar + arenaInt[0];
				} else {
					arenaChar2 = arenaChar1;
				}
				
				arenaInt = (int*)arenaChar0;/*inainte de a iesi din do while, va pointa catre
											 inceputul ultimului bloc de memorie*/
			} else if (arenaChar2 - arenaChar1 >= size + 3 * sizeof(int)) {/*in caz ca s-a gasit
																			o zona libera de memorie suficient de mare*/
				arenaInt = (int*)arenaChar1;//configuram sectiunea de gestiune pentru noul bloc de memorie
				arenaInt[0] = arenaChar2 - arenaChar;
				arenaInt[1] = arenaChar0 - arenaChar;
				arenaInt[2] = size + 3 * sizeof(int);

				arenaInt = (int*)arenaChar0;/*modificam sectiunea de gestiune a
											blocului de memorie precedent*/
				arenaInt[0] = arenaChar1 - arenaChar;

				arenaInt = (int*)arenaChar2;/*modificam sectiunea de gestiune a 
											blocului de memorie posterior*/
				arenaInt[1] = arenaChar1 - arenaChar;

				alocareEfectuata = 1;
				printf("%d\n", arenaChar1 - arenaChar + 3 * sizeof(int));
				return arenaChar1 - arenaChar + 3 * sizeof(int);
			} else {//daca nu s-a gasit o zona libera de memorie suficient de mare
				arenaChar0 = arenaChar2;//se continua cautarea
				arenaInt = (int*)arenaChar0;
				arenaChar1 = arenaChar2 + arenaInt[2];
				if (arenaInt[0] != 0) {
					arenaChar2 = arenaChar + arenaInt[0];
				} else {
					arenaChar2 = arenaChar1;
				}
			}
		} while (alocareEfectuata == 0 && arenaInt[0] != 0);/*cautarea se va opri cand 
															alocarea a avut loc sau cand sectiunea de gestiune 
															a ultimului bloc va avea pe prima pozitie 0*/

		if (alocareEfectuata == 0) {/*daca nu s a gasit nicio zona libera de memorie pana
								 la ultimul bloc alocat, cautam o zona de memorie libera intre ultimul bloc si sfarsit*/
			if(N - (arenaChar1 - arenaChar) >= size + 3 * sizeof(int)){/*daca exista memorie
																	 libera pana la sfarsitul arenei*/
				arenaInt = (int*)arenaChar1;
				arenaInt[0] = 0;//configurez sectiunea de gestiune a noului bloc
				arenaInt[1] = arenaChar0 - arenaChar;
				arenaInt[2] = size + 3 * sizeof(int);

				arenaInt = (int*)arenaChar0;//configurez sectiunea de gestiune a blocului precedent
				arenaInt[0] = arenaChar1 - arenaChar;

				printf("%d\n", arenaChar1 - arenaChar + 3 * sizeof(int));//afisez indexul sectiunii de date
				return arenaChar1 - arenaChar + 3 * sizeof(int);
			} else {//daca nu exista memorie libera pana la sfarsitul arenei
				printf("%d\n", 0);
				return 0;
			}
		}
	}
}

void FREE (unsigned char *arenaChar, int index) {
	unsigned char *arChar;
	int *arenaInt, adresaInainte, adresaDupa;

	arChar = arenaChar + index;//trece la indexul sectiunii de date ce are sa fie eliberata
	arenaInt = (int*)arChar;/*preiau din zona de gestiune a blocului de date ce are 
							sa fie eliberat adresa blocului precedent si urmator*/
	arenaInt -= 3;
	adresaInainte = arenaInt[1];
	adresaDupa = arenaInt[0];

	arChar = arenaChar + adresaInainte;//actualizez sectiunea de gestiune a blocului precedent
	arenaInt = (int*)arChar;
	arenaInt[0] = adresaDupa;

	if (adresaDupa != 0) {//actualizez sectiunea de gestiune a blocului urmator daca exista
		arChar = arenaChar + adresaDupa;
		arenaInt = (int*)arChar;
		arenaInt[1] = adresaInainte;
	}
}

void FILL (unsigned char *arenaChar, int index, int size, int value) {
	unsigned char *arChar;
	arChar = arenaChar + index;//trece la indexul sectiunii de date ce va fi completata

	while (size != 0) {//cat timp nu s-a ajuns la sfarsitul sectiunii de date
		size--;
		*arChar = value;
		arChar++;
	}
}

void SHOWFREE (unsigned char *arenaChar, int N) {
	int *arenaInt = (int*)arenaChar;
	if (*arenaInt == 0) {//daca nu exista niciun bloc alocat
		printf("1 blocks (%d bytes) free\n", N - 4);
	} else {
		unsigned char *arenaChar0 = arenaChar,/*arenaChar0 va retine adresa inceputului 
											sectiunii de gestiune a blocului care precede zona de memorie libera analizata*/
						*arenaChar1 = arenaChar,//arenaChar1 va retine adresa inceputului zonei de memorie libera
						*arenaChar2 = arenaChar;/*arenaChar2 va retine adresa sfaristului zonei de memorie libera
												va avea aceeasi valoare ca arenaChar1 in cazul in care nu s-a gasit
												 o zona de momerie libera*/
		int nrFreeBlocks,//va retine numarul de blocuri libere
			nrFreeBytes;//va retine numarul de bytes liberi
		
		arenaChar1 += sizeof(int);
		arenaChar2 += arenaInt[0];
		
		do {
			if (arenaChar2 - arenaChar1 == 0) {//daca nu s-a gasit o zona libera de memorie
				arenaChar0 = arenaChar1;//cauta in continuare
				arenaInt = (int*)arenaChar1;
				arenaChar1 = arenaChar2 + arenaInt[2];
				if (arenaInt[0] != 0) {
					arenaChar2 = arenaChar + arenaInt[0];
				} else {
					arenaChar2 = arenaChar1;
				}
				arenaInt = (int*)arenaChar0;
			} else {//daca s-a gasit o zona libera
				nrFreeBlocks++;//creste numarul de zone  libere
				nrFreeBytes += arenaChar2 - arenaChar1;//creste numarul de bytes liberi

				arenaChar0 = arenaChar2;//se cauta in continuare
				arenaInt = (int*)arenaChar0;
				arenaChar1 = arenaChar2 + arenaInt[2];
				arenaChar2 = arenaChar + arenaInt[0];	
			}
		} while (arenaInt[0] != 0);/*cautarea se va opri cand, in sectiunea de gestiune 
									a ultimului bloc, pe prima pozitie va fi 0*/

		if (arenaChar + N - arenaChar1 != 0) {/*daca mai exista memorie 
												libera pana la sfarsitul arenei*/
			nrFreeBlocks++;//creste numarul de blocuri libere
			nrFreeBytes += arenaChar + N - arenaChar1;//creste numarul de bytes liberi
		}

		printf("%d blocks (%d bytes) free\n", nrFreeBlocks, nrFreeBytes);/*afisarea
																 numarului de blocuri si bytes liberi*/
	}
}

void SHOWUSAGE (unsigned char *arenaChar, int N) {
	int *arenaInt = (int*)arenaChar, efficiency, fragmentation;
	if (*arenaInt == 0) {//daca nu exista niciun bloc alocat
		printf("0 blocks (0 bytes) used\n0%% efficiency\n0%% fragmentation\n");
	} else {
		unsigned char *arenaChar0 = arenaChar,/*arenaChar0 va retine adresa inceputului 
											sectiunii de gestiune a blocului care precede zona de memorie libera analizata*/
						*arenaChar1 = arenaChar,//arenaChar1 va retine adresa inceputului zonei de memorie libera
						*arenaChar2 = arenaChar;/*arenaChar2 va retine adresa sfaristului zonei de memorie libera
												va avea aceeasi valoare ca arenaChar1 in cazul in care nu s-a gasit
												 o zona de momerie libera*/
		int nrFreeBlocks = 0,//va retine numarul de blocuri libere
			nrFreeBytes = 0,//va retine numarul de bytes liberi
			nrUsedBlocks = 0,//va retine numarul de blocuri ocupate
			nrUsedBytes = 0;//va retine numarul de bytes ocupati

		
		arenaChar1 += sizeof(int);
		arenaChar2 += arenaInt[0];
		
		do {
			if (arenaChar2 - arenaChar1 == 0) {//daca nu s-a gasit o zona libera de memorie
				arenaChar0 = arenaChar1;//se cauta in continuare
				arenaInt = (int*)arenaChar1;
				arenaChar1 = arenaChar2 + arenaInt[2];
				
				if (arenaInt[0] != 0) {
					arenaChar2 = arenaChar + arenaInt[0];
				} else {
					arenaChar2 = arenaChar1;
				}
				
				arenaInt = (int*)arenaChar0;
				nrUsedBlocks++;//crestem numarul de blocuri ocupate
				nrUsedBytes += arenaInt[2] - 3 * sizeof(int);//crestem numarul de bytes ocupati
			} else {//daca s-a gasit o zona libera
				nrFreeBlocks++;//creste numarul de blocuri libere
				nrFreeBytes += arenaChar2 - arenaChar1;//creste numarul de bytes liberi

				arenaChar0 = arenaChar2;//se cauta in continuare
				arenaInt = (int*)arenaChar0;
				arenaChar1 = arenaChar2 + arenaInt[2];
				arenaChar2 = arenaChar + arenaInt[0];

				nrUsedBlocks++;//creste numarul de blocuri ocupate
				nrUsedBytes += arenaInt[2] - 3 * sizeof(int);//creste numarul de bytes ocupati
	
			}
		} while (arenaInt[0] != 0);/*cautarea se va opri cand, in sectiune de gestiune 
									a ultimului bloc, pe prima pozitie va fi 0*/

		if (arenaChar + N - arenaChar1 != 0) {/*daca mai exista memorie libera pana la 
												sfarsitul arenei*/
			nrFreeBlocks++;//creste numarul de blocuri libere
			nrFreeBytes += arenaChar + N - arenaChar1;//creste numarul de bytes liberi
		}

		efficiency = 100 * nrUsedBytes / (N - nrFreeBytes);//calculeaza eficienta
		fragmentation = 100 * (nrFreeBlocks - 1) / nrUsedBlocks;//calculeaza fragmentarea
		printf("%d blocks (%d bytes) used\n%d%% efficiency\n%d%% fragmentation\n",
				 nrUsedBlocks, nrUsedBytes, efficiency, fragmentation);
	}
}

void SHOWALLOCATIONS (unsigned char *arenaChar, int N) {
	int *arenaInt = (int*)arenaChar;
	if (*arenaInt == 0) {
		printf("OCCUPIED 4 bytes\n");
		printf("FREE %d bytes\n", N - 4);
	} else {
		printf("OCCUPIED 4 bytes\n");
		unsigned char *arenaChar0 = arenaChar,/*arenaChar0 va retine adresa inceputului
											sectiunii de gestiune a blocului care precede zona de memorie libera analizata*/
						*arenaChar1 = arenaChar,//arenaChar1 va retine adresa inceputului zonei de memorie libera
						*arenaChar2 = arenaChar;/*arenaChar2 va retine adresa sfaristului zonei de memorie libera
											va avea aceeasi valoare ca arenaChar1 in cazul in care nu s-a gasit
											 o zona de momerie libera*/
		
		arenaChar1 += sizeof(int);
		arenaChar2 += arenaInt[0];
		
		do {
			if (arenaChar2 - arenaChar1 == 0) {//daca nu s-a gasit o zona libera
				arenaChar0 = arenaChar1;//se cauta in continuare
				arenaInt = (int*)arenaChar1;
				arenaChar1 = arenaChar2 + arenaInt[2];
				if (arenaInt[0] != 0) {
					arenaChar2 = arenaChar + arenaInt[0];
				} else {
					arenaChar2 = arenaChar1;
				}
				printf("OCCUPIED %d bytes\n", arenaInt[2]);//afiseaza numarul de bytes ocupati
				arenaInt = (int*)arenaChar0;
			} else {//daca s-a gasit o zona libera
				printf("FREE %d bytes\n", arenaChar2 - arenaChar1);//afiseaza numarul de bytes liberi

				arenaChar0 = arenaChar2;//se cauta in continuare
				arenaInt = (int*)arenaChar0;
				arenaChar1 = arenaChar2 + arenaInt[2];
				arenaChar2 = arenaChar + arenaInt[0];	

				printf("OCCUPIED %d bytes\n", arenaInt[2]);//afiseaza numarul de bytes ocupati
			}
		} while (arenaInt[0] != 0);

		if (arenaChar + N - arenaChar1 != 0) {/*daca mai exista memorie libera pana la 
												sfarsitul arenei*/
			printf("FREE %d bytes\n", arenaChar + N - arenaChar1);//se afiseaza numarul de bytes liberi
		}
	}
}

void ALLOCALIGNED (unsigned char *arenaChar, int size, int align, int N) {
	int *arenaInt = (int*)arenaChar, multiplu, alignDup;
	if (arenaInt[0] == 0) {

		multiplu = 1;
		alignDup = align;
		while (alignDup - 16 < 0) {/*daca align are o valoare mai mica decat 4 * sizeof(int)
								 (pentru sectiunea de gestiune a noului bloc si indexul de start)*/
			multiplu++;
			alignDup = align * multiplu;
		}

		arenaInt[0] = alignDup - 3 * sizeof(int);
		arenaInt += alignDup / sizeof(int);
		arenaInt -= 3;
		arenaInt[0] = 0;
		arenaInt[1] = 0;
		arenaInt[2] = size + 3 * sizeof(int);
		printf("%d\n", alignDup);
	} else {
		unsigned char *arenaChar0,/*arenaChar0 va retine adresa inceputului sectiunii
								 de gestiune a blocului care precede zona de memorie libera analizata*/
						*arenaChar1,//arenaChar1 va retine adresa inceputului zonei de memorie libera
						*arenaChar2;/*arenaChar2 va retine adresa sfaristului zonei de memorie libera
									va avea aceeasi valoare ca arenaChar1 in cazul in care nu s-a gasit
									 o zona de momerie libera*/
		int alocareEfectuata = 0; //presupunem ca alocarea nu a avut loc

		multiplu = 1;
		do {	
			alignDup = align * multiplu;

			while (alignDup < 3 * sizeof(int)) {/*daca align are o valoare prea mica, 
												se va cauta primul multiplu mai mare decat 3 * sizeof(int)*/
				multiplu++;
				alignDup = align * multiplu;
			}

			arenaInt = (int*)arenaChar;//se cauta de fiecare data de la inceput
			arenaChar0 = arenaChar;
			arenaChar1 = arenaChar + sizeof(int);
			arenaChar2 = arenaChar + arenaInt[0];

			do {
				if (arenaChar2 - arenaChar1 == 0) {
					arenaChar0 = arenaChar1;
					arenaInt = (int*)arenaChar1;
					arenaChar1 = arenaChar2 + arenaInt[2];
					if (arenaInt[0] != 0) {
						arenaChar2 = arenaChar + arenaInt[0];
					} else {
						arenaChar2 = arenaChar1;
					}
				
					arenaInt = (int*)arenaChar0;/*inainte de a iesi din do while, va pointa 
												catre inceputul ultimului bloc de memorie*/
				} else if (arenaChar2 - arenaChar1 >= size + 3 * sizeof(int) && 
							(arenaChar1 - arenaChar) <= alignDup - 3 * sizeof(int) &&
							(arenaChar2 - arenaChar) >= alignDup + size) {
							//in caz ca s-a gasit o zona libera de memorie suficient de mare

					arenaInt = (int*)arenaChar;
					arenaInt += alignDup / sizeof(int);
					arenaInt -= 3;
					arenaInt[0] = arenaChar2 - arenaChar;//configurez sectiunea de gestiune a noului bloc
					arenaInt[1] = arenaChar0 - arenaChar;
					arenaInt[2] = size + 3 * sizeof(int);

					arenaInt = (int*)arenaChar0;//modificam sectiunea de gestiune a blocului de memorie precedent
					arenaInt[0] = alignDup - 12;

					arenaInt = (int*)arenaChar2;//modificam sectiunea de gestiune a blocului de memorie posterior
					arenaInt[1] = alignDup - 12;

					alocareEfectuata = 1;
					printf("%d\n", alignDup);
				} else {//daca nu s-a gasit o zona libera de memorie suficient de mare, o cautam pe urmatoarea
					arenaChar0 = arenaChar2;
					arenaInt = (int*)arenaChar0;
					arenaChar1 = arenaChar2 + arenaInt[2];
					if (arenaInt[0] != 0) {
						arenaChar2 = arenaChar + arenaInt[0];
					} else {
						arenaChar2 = arenaChar1;
					}
				}
			} while (alocareEfectuata == 0 && arenaInt[0] != 0);

			multiplu++;
			if (alocareEfectuata == 1) {
				break;//daca s-a gasit zona libera si alocarea a avut loc, se iese din do while
			}

		} while (alignDup - 3 * sizeof(int) < arenaChar1 - arenaChar);/*se repeta procesul
		 pana cand alignDup devine destul de mare pentru a trece de ultimul bloc de momerie
		 si sa fie si destula memorie pentru zona de gestiune; asta in caz ca nu s-a gasit 
		 loc liber pana la ultimul bloc*/

		if (alocareEfectuata == 0) {/*daca nu s a gasit nicio zona libera de memorie pana
									 la ultimul bloc alocat, cautam o zona de memorie libera 
									 intre ultimul bloc si sfarsit*/
			if(N - (arenaChar1 - arenaChar) >= size + 3 * sizeof(int) && alignDup + size <= N){
						//daca exista memorie libera pana la sfarsit
					arenaInt = (int*)arenaChar;
					arenaInt += alignDup / sizeof(int);
					arenaInt -= 3;
					arenaInt[0] = 0;//configurez sectiunea de gestiune a noului bloc
					arenaInt[1] = arenaChar0 - arenaChar;
					arenaInt[2] = size + 3 * sizeof(int);

					arenaInt = (int*)arenaChar0;//actualizez sectiunea de gestiune a blocului precedent
					arenaInt[0] = alignDup - 3 * sizeof(int);

					printf("%d\n", alignDup); 
			} else {
				printf("%d\n", 0);
			}
		}
	}
}

void REALLOC (unsigned char* arenaChar, int oldIndex, int newSize, int N) {
	int newIndex, oldSize, *arenaInt, i;
	unsigned char *arenaCharOld, *arenaCharNew;

	arenaCharOld = arenaChar + oldIndex;/*aflam dimensiunea sectiunii de date 
											a blocului ce are sa fie realocat*/
	arenaInt = (int*)arenaCharOld;
	arenaInt--;
	oldSize = arenaInt[0] - 3 * sizeof(int);

	FREE(arenaChar, oldIndex);/*se elibereaza blocul de memorie care are sectiunea 
								de date la indexul dat*/
	newIndex = ALLOC(arenaChar, newSize, N);/*se aloca un nou bloc de memorie cu 
												dimensiunea data*/

	arenaCharNew = arenaChar + newIndex;
	if (newSize < oldSize) {/*daca dimensiunea blocului este mai mica 
							decat cea dinaintea realocarii*/
		for (i = 0; i < newSize; i++) {//vor fi copiati doar size octeti de date
			arenaCharNew[i] = arenaCharOld[i];
		}
	} else {//altfel
		for (i = 0; i < oldSize; i++) {//vor fi copiati toti octetii de date
			arenaCharNew[i] = arenaCharOld[i];
		}
	}
}

void SHOWMAP (unsigned char* arenaChar, int length, int N) {
	int *arenaInt = (int*)arenaChar, i, j,
		numarElemente = 0, vectorZoneLibereOcupate[N],/*vectorul va corespunde cu octetii
							 liberi si ocupati din arena: va contine 1 pentru un byte ocupat si 0 pentru unul liber*/
		numarCaractere = 0;
	float x, y = 0;
	
	if (arenaInt[0] == 0) {
		for (i = 0; i < N; i++) {
			if (i < 4) {
				vectorZoneLibereOcupate[i] = 1;
			} else {
				vectorZoneLibereOcupate[i] = 0;
			}
		}
		numarElemente = N;
	} else { 
		unsigned char *arenaChar0 = arenaChar, /*arenaChar0 va retine adresa inceputului
											sectiunii de gestiune a blocului care precede zona de memorie libera analizata*/
						*arenaChar1 = arenaChar, //arenaChar1 va retine adresa inceputului zonei de memorie libera
						*arenaChar2 = arenaChar;/*arenaChar2 va retine adresa sfaristului zonei de memorie libera
												va avea aceeasi valoare ca arenaChar1 in cazul in care nu s-a gasit
												 o zona de momerie libera*/

		arenaChar1 += sizeof(int);
		arenaChar2 += arenaInt[0];
		
		do {
			if (arenaChar2 - arenaChar1 == 0) {//daca nu s-a gasit o zona libera
				for (i = 0; i < arenaChar1 - arenaChar0; i++) {/*se adauga 1 in vector pentru 
																	octetii ocupati*/
					vectorZoneLibereOcupate[numarElemente++] = 1;
				}

				arenaChar0 = arenaChar1;
				arenaInt = (int*)arenaChar1;
				arenaChar1 = arenaChar2 + arenaInt[2];
				if (arenaInt[0] != 0) {
					arenaChar2 = arenaChar + arenaInt[0];
				} else {
					arenaChar2 = arenaChar1;
				}
				
				arenaInt = (int*)arenaChar0;/*inainte de a iesi din do while, va pointa catre 
												inceputul ultimului bloc de memorie*/ 
			} else {//daca s-a gasit o zona libera de memorie 
				for (i = 0; i < arenaChar1 - arenaChar0; i++) {/*se adauga 1 in vector pentru 
															octetii ocupati, dinaintea zonei libere*/
					vectorZoneLibereOcupate[numarElemente++] = 1;
				}
				for (i = 0; i < arenaChar2 - arenaChar1; i++) {/*se adauga 0 in vector pentru
																 octetii liberi*/
					vectorZoneLibereOcupate[numarElemente++] = 0;
				}

				arenaChar0 = arenaChar2;
				arenaInt = (int*)arenaChar0;
				arenaChar1 = arenaChar2 + arenaInt[2];
				if (arenaInt[0] != 0) {
					arenaChar2 = arenaChar + arenaInt[0];
				} else {
					arenaChar2 = arenaChar1;
				}
			}
		} while (arenaInt[0] != 0);

		for (i = 0; i < (arenaChar1 - arenaChar0); i++) {/*se adauga 1 in vector pentru 
															ultimul bloc de memorie*/
			vectorZoneLibereOcupate[numarElemente++] = 1;
		}

		if(N - (arenaChar1 - arenaChar) > 0){//daca exista memorie libera pana la sfarsit
			for (i = 0; i < (N - (arenaChar1 - arenaChar)); i++) {//se adauga 0 in vector
				vectorZoneLibereOcupate[numarElemente++] = 0;
			}
		}
	}
	
	int xBytesOccupied = 0;
	x = N * 1.0 / length;

	if (x >= 1) {//daca x nu e subunitar

		for(i = 0; i < N; i++) {//y retine numarul de octeti care au fost luati in calcul
			if(y + 1 >= x) {//cand s-a ajuns la limita de x bytes, se trece mai departe
				y = y + 1 - x; 
				if (vectorZoneLibereOcupate[i] == 1) { 
					xBytesOccupied = 1; 
				}
				if (xBytesOccupied == 1) {//daca vreunul dintre ce x bytes este 1
					printf("*");
					numarCaractere++;
					if (numarCaractere % 80 == 0) {/*se afiseaza o linie noua daca s-a atins un 
															multiplu de 80 de caractere*/
						printf("\n");
					}
				} else {//daca toti cei x bytes sunt 0
					printf(".");
					numarCaractere++;
					if (numarCaractere % 80 == 0) {
						printf("\n");
					}
                }
				
				xBytesOccupied = 0;//se reseteaza xBytesOccupied

				if (y != 0) {/*se cauta urmatorii x bytes. daca din octetul de pe pozitia i a 
							mai ramas o portiune, se verifica valoarea sa si, daca este 1,
							se retine in xBytesOccupied*/
					if (vectorZoneLibereOcupate[i] == 1) { 
						xBytesOccupied = 1; 
					}
				}
			} else {
				y += 1;//daca nu s-a ajuns la la limita de x bytes
				if (vectorZoneLibereOcupate[i] == 1) { 
					xBytesOccupied = 1; 
				}
			}
		}

		if (y > 1 && numarCaractere != length) {//pentru caracterul care a fost ignorat pe final
			y = y + 1 - x; 
				if (vectorZoneLibereOcupate[i] == 1) { 
					xBytesOccupied = 1; 
				}
				if (xBytesOccupied == 1) {
					printf("*");
					numarCaractere++;
					if (numarCaractere % 80 == 0) {
						printf("\n");
					}
				} else {
					printf(".");
					numarCaractere++;
					if (numarCaractere % 80 == 0) {
						printf("\n");
					}
                                }
					xBytesOccupied = 0;

					if (y != 0) {
						if (vectorZoneLibereOcupate[i] == 1) {
							xBytesOccupied = 1;
						}
					}
		}

		if (numarCaractere % 80 != 0) {
			printf("\n");
		}
	} else {//daca x este subunitar
		for(i = 0; i < N; i++) {
			while ((y + x) <= 1) {
				y += x;//y retine numarul de octeti care au fost luati in calcul

				if (vectorZoneLibereOcupate[i] == 1) {
					printf("*");
					numarCaractere++;
					if(numarCaractere % 80 == 0) {
						printf("\n");
					}
				} else {
					if (i != 0 && xBytesOccupied == 1){
						printf("*");
						numarCaractere++;
						if(numarCaractere % 80 == 0) {
							printf("\n");
					}
						xBytesOccupied = 0;
					} else {		
						printf(".");
						numarCaractere++;
						if(numarCaractere % 80 == 0) {
							printf("\n");
						}
					}
				}
			}
			y -= 1;/*va avea valoare negativa, pentru a sti cat trebuie sa ia in considerare
					 din urmatorul octet*/
			if (y != 0) {
				if (vectorZoneLibereOcupate[i] == 1) xBytesOccupied = 1;
			}
	
		}

		if ((y + x) > 0 && numarCaractere != length) {//pentru caracterul ignorat pe final
			if (xBytesOccupied == 0) {
				printf(".");
				numarCaractere++;
				if(numarCaractere % 80 == 0) printf("\n");
			} else {
				printf("*");
				numarCaractere++;
				if(numarCaractere % 80 == 0) printf("\n");
			}
		}
	
		if (numarCaractere % 80 != 0) {
			printf("\n");
		}
	}
}

int main () {
	int N, size, index, value, align, length;
	unsigned char *arenaChar;
	char stringCitire[30], *comanda1, *comanda2;

	while (1) {//se citesc comenzi pana la intalnirea comenzii FINALIZE
		fgets(stringCitire, 30, stdin);/*string-ul citit va fi spart in bucati,
										 care vor fi analizate*/
		comanda1 = strtok(stringCitire, " ");

		if(strcmp(comanda1, "INITIALIZE") == 0) {
			comanda2 = strtok(NULL, " ");
			N = atoi(comanda2);
			INITIALIZE (N, &arenaChar);
		} if (strcmp(comanda1, "FINALIZE\n") == 0) {
			FINALIZE (arenaChar);
		} else if (strcmp(comanda1, "DUMP\n") == 0) {
			DUMP (arenaChar, N);
		} else if (strcmp(comanda1, "ALLOC") == 0) {
			comanda2 = strtok(NULL, " ");
			size = atoi(comanda2);
			ALLOC(arenaChar, size, N);
		} else if (strcmp(comanda1, "FREE") == 0) {
			comanda2 = strtok(NULL, " ");
			index = atoi(comanda2);
			FREE(arenaChar, index);
		} else if (strcmp(comanda1, "FILL") == 0) {
			comanda2 = strtok(NULL, " ");
			index = atoi(comanda2);
			comanda2 = strtok(NULL, " ");
			size = atoi(comanda2);
			comanda2 = strtok(NULL, " ");
			value = atoi(comanda2);
			FILL(arenaChar, index, size, value);
		} else if (strcmp(comanda1, "SHOW") == 0) {
			comanda2 = strtok(NULL, " ");
			if (strcmp(comanda2, "FREE\n") == 0) {
				SHOWFREE(arenaChar, N);
			} else if (strcmp(comanda2, "USAGE\n") == 0) {
				SHOWUSAGE(arenaChar, N);
			} else if (strcmp(comanda2, "ALLOCATIONS\n") == 0) {
				SHOWALLOCATIONS(arenaChar, N);
			} else if (strcmp(comanda2, "MAP") == 0) {
				comanda2 = strtok(NULL, " ");
				length = atoi(comanda2);
				SHOWMAP(arenaChar, length, N);
			}
		} else if (strcmp(comanda1, "ALLOCALIGNED") == 0) {
			comanda2 = strtok(NULL, " ");
			size = atoi(comanda2);
			comanda2 = strtok(NULL, " ");
			align = atoi(comanda2);
			ALLOCALIGNED(arenaChar, size, align, N);
		} else if (strcmp(comanda1, "REALLOC") == 0) {
			comanda2 = strtok(NULL, " ");
			index = atoi(comanda2);
			comanda2 = strtok(NULL, " ");
			size = atoi(comanda2);
			REALLOC(arenaChar, index, size, N);
		}
	}

	return 0;
}