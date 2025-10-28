/*
------------------------------------------------------------
    Programa: CREW Search Secuencial
    Lenguaje: C
    Descripción:
        Implementación secuencial del algoritmo CREW_SEARCH
        (Concurrent Read, Exclusive Write), que simula una
        búsqueda binaria en paralelo.

        El algoritmo divide la secuencia ordenada en bloques
        manejados por "procesadores virtuales" y, en cada etapa,
        ajusta los límites de búsqueda según el resultado de
        comparaciones locales.

        En esta versión:
            - Se genera dinámicamente un arreglo ordenado.
            - La búsqueda se realiza secuencialmente.
            - Se señalan las partes que podrían paralelizarse.

    Nota:
        El modelo CREW permite lecturas simultáneas (Read) pero
        no escrituras concurrentes (Write). Por eso algunas
        variables son "compartidas" y deben protegerse.
------------------------------------------------------------
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>


/* ------------------------------------------------------------
   Función: pow_int
   Descripción:
       Calcula potencias enteras sin usar la función pow()
       para evitar problemas con conversiones de double a int
       y exponentes negativos.
   Parámetros:
       base → número base
       exp → exponente (entero positivo)
   Retorna:
       base elevado a exp (entero)
------------------------------------------------------------ */

int pow_int(int base, int exp) {
    int result = 1;
    for (int i = 0; i < exp; i++) {
		result *= base;
	}
    return result;
}

/* ------------------------------------------------------------
   Función: crew_search
   Descripción:
       Implementa el algoritmo CREW_SEARCH de forma secuencial.
       Dada una secuencia ordenada, busca un elemento aplicando
       la lógica de división en paralelo del modelo CREW PRAM.

   Parámetros:
       secuencia[] → arreglo ordenado de enteros.
       tamanoSecuencia → tamaño total del arreglo.
       elementoBuscado → valor a localizar dentro de la secuencia.
       numProcesadores → número de procesadores simulados.

   Retorno:
       No retorna valor. Muestra en pantalla si el elemento fue
       encontrado y su posición.
------------------------------------------------------------ */

void crew_search(int secuencia[], int tamañoSecuencia, int elementoBuscado, int numProcesadores) {
	/*Inicio de los limites y variables compartidas.*/
	int indiceInicio = 1; //El valor aca dira en donde se inicia la subsecuencia.
	int indiceFin = tamañoSecuencia; //Aquí el valor nos indicara el final de la subsecuencia.
	int posicionEncontrada = 0; //Posición del elemento encontrado (si no esta en la secuencia este devolvera cero).
	int numEtapas = ceil(log2(tamañoSecuencia + 1) / log2(numProcesadores + 1));
	//Aquí se definen las variables que van a indicar el sentido en que se hara la busqueda.
	char direccionInicial = 'R'; //Aqui se posiciona para buscar hacia la derecha.
	char direccionFinal = 'L'; //Se posiciona para buscar hacia el lado izquierdo.

	printf("q = %d\nr = %d\n", indiceInicio, indiceFin);
	printf("c[0] = %c, c[%d] = %c\n", direccionInicial, numProcesadores, direccionFinal);
	printf("k = %d, g = %d\n\nEntra al while\n",posicionEncontrada, numEtapas);

	//Bucle principal de busqueda.
	while (indiceInicio <= indiceFin && posicionEncontrada == 0) {
		//Si ya no quedan etapas validas entonces salimos del programa.
		if (numEtapas <= 0) {
			break;
		}

		int fronteraInicial = indiceInicio - 1; //Frontera del primer procesador. 
		char direccionBusqueda[numProcesadores + 2]; //Arreglo de los valores pivotes que seran asignados con 'R' o 'L'.
		int fronteraProcesador[numProcesadores + 1]; //Indices frontera.

		fronteraProcesador[0] = fronteraInicial;
		fronteraProcesador[numProcesadores +1] = indiceFin-1;

		direccionBusqueda[0] = direccionInicial;
		direccionBusqueda[numProcesadores + 1] = direccionFinal;
		//Aquí se calcula el numero de saltos, osea cuantas rondas debe de hacer el algoritmo para hallar la seccion en donde se encuentre el valor que queremos.
		int paso = pow_int(numProcesadores + 1, numEtapas - 1);


		printf("arreglo actual\n{");
		for(int i = indiceInicio -1; i<indiceFin; i++){
			printf((i == indiceFin-1)?"%d":"%d, ", secuencia[i]);
		}	
		printf("}\n");

		printf("j[%d] = %d", 0, fronteraProcesador[0]);
		printf(" : c[%d] = %c\n", 0, direccionBusqueda[0]);
		/* --------------------------------------------------------
           SECCIÓN PARALELIZABLE
           En una versión paralela, cada procesador Pi ejecutaría
           este bloque de forma simultánea, comparando su elemento
           frontera y asignando su dirección de búsqueda (L o R).
           -------------------------------------------------------- */
		int endReached = 0;
		for (int i = 1; i <= numProcesadores; i++) {
			//Calcula la posición frontera de cada procesador en la secuencia.
			fronteraProcesador[i] = (indiceInicio - 1) + i * paso;
			if (fronteraProcesador[i] <= indiceFin) {
				int indice = fronteraProcesador[i] - 1;
				//Aquí compara el elemento en la frontera con el valor que queremos buscar.
				if (secuencia[indice] == elementoBuscado) {
					posicionEncontrada = fronteraProcesador[i];
				} else if (secuencia[indice] > elementoBuscado) {
					direccionBusqueda[i] = 'L'; //Busca a la izquierda.
				} else {
					direccionBusqueda[i] = 'R'; //Busca a la derecha.
				}
			} else {
				//Si la frontera se pasa del rango, se retrocede y se busca hacia la izquierda.
				fronteraProcesador[i] = indiceFin - 1;
				direccionBusqueda[i] = 'L';
			}

			if(endReached) continue;

			endReached = (fronteraProcesador[i-1] == fronteraProcesador[i]);
			printf("j[%d] = %d :", i, fronteraProcesador[i-1]+1);
			printf(" c[%d] = ", i);
			printf((endReached || i == numProcesadores)?"%c":"%c\n", direccionBusqueda[i]);
		}


		// --FIN DE LA SECCION PARALELA.

		/* --------------------------------------------------------
           ACTUALIZACIÓN DE LÍMITES
           Solo un procesador (el que detecta el cambio de dirección)
           actualiza los valores compartidos q y r (indiceInicio y indiceFin).
           -------------------------------------------------------- */

		//Se actualizan los valores de la busqueda.
		for (int i = 1; i <= numProcesadores; i++) {
			//Si hay un cambio de dirección ya sea (derecha o izquierda), se ajusta el rango.
			if (direccionBusqueda[i] != direccionBusqueda[i - 1]) {
				indiceInicio = fronteraProcesador[i - 1] + 1;

				indiceFin = fronteraProcesador[i] - 1;
				break;
			}
			//Caso especial, aquí es donde el ultimo procesador es diferente al ficticio final.
			if (i == numProcesadores && direccionBusqueda[numProcesadores] != direccionFinal) {
				indiceInicio = fronteraProcesador[i] + 1;
			}
		}
		//Se van reduciendo las busquedas en cada iteración, así se hace mas pequeña el area de busqueda.
		numEtapas--;
		printf("\ng = %d\n\n", numEtapas);
	}

	// --Resultado--
	if (posicionEncontrada != 0) {
		printf("Elemento %d encontrado en la posicion %d\n", elementoBuscado, posicionEncontrada);
	} else {
		printf("Elemento %d no encontrado en la secuencia.\n", elementoBuscado);
	}
}

/* ------------------------------------------------------------
   Función principal (main)
   Descripción:
       Solicita al usuario el tamaño del arreglo a generar,
       construye una secuencia ordenada y ejecuta el algoritmo
       CREW_SEARCH de forma secuencial.

       Se incluyen validaciones de rango y control de errores
       en la reserva de memoria.
------------------------------------------------------------ */

int main() {
	int numeroValores;
	int *secuencia;

	//Se verifica que el valor entregado si se encuentre entre los rangos pedidos.
	printf("Ingresa un N para generar la secuencia de numeros.\n");
	scanf("%d", &numeroValores);
	while (numeroValores < 16 || numeroValores > 500) {
		printf("Error: el número debe estar entre 16 y 500.\n");
		printf("Ingrese nuevamente un N:\n");
		scanf("%d", &numeroValores);
	}
	//Asignación de memoria dinamica del arreglo 'Secuencia'.
	secuencia = (int *)malloc(numeroValores*sizeof(int));
	if (secuencia == NULL) {
		printf("Error: No se pudo asignar la memoria para el arreglo.\n");
		return -1;
	}
	for (int i = 0; i < numeroValores; i++) {
		secuencia[i] = i+1;
	}

	//int secuencia[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
	//Parametros de busqueda.
	int tamañoSecuencia = numeroValores;
	int elementoBuscado = 23;
	int numProcesadores = 10;

	//verificación con respecto a que si el valor buscado supera el tamaño del arreglo, entonces no los busque porque no lo encontrara.
	if (elementoBuscado >= tamañoSecuencia) {
		printf("El elemento %d no puede buscarse porque no se encuentra en la secuencia generada (0 a %d).\n", elementoBuscado, tamañoSecuencia - 1);
		free(secuencia);
		return 0; 
	}

	//Ejecutamos el algoritmo.
	crew_search(secuencia, tamañoSecuencia, elementoBuscado, numProcesadores);
	//Liberamos la memoria dinamica.
	free(secuencia);
	return 0;
}
