#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#define MAX_Q_SIZE 100000000

typedef struct QuadtreeNode{
	unsigned char blue, green, red; 
	uint32_t area;
	int32_t top_left, top_right; 
	int32_t bottom_left, bottom_right;
}__attribute__ ((packed)) QuadtreeNode;

typedef struct QNode{
	unsigned char blue, green, red; 
	uint32_t area;
	int32_t index;
	struct QNode* top_left;
	struct QNode* top_right;
	struct QNode* bottom_left;
	struct QNode* bottom_right;
}qnode;

typedef struct RGB{
	unsigned char blue, green, red;
}RGB;

/***FUNCTIA freeMat*** -> elibereaza memoria unei matrici de pixeli*/
void freeMat(RGB** rgb, uint32_t size){
	int i = 0;
	for(i = 0; i < size; i++)
		free(rgb[i]);
	free(rgb);
}

/***FUNCTIA freeQtree*** -> elibereaza memoria unui quadtree*/
void freeQTree(qnode* root){
	if(root == NULL)
		return;

	freeQTree(root->top_left);
	freeQTree(root->top_right);
	freeQTree(root->bottom_right);
	freeQTree(root->bottom_left);

	free(root);
}

/***FUNCTIA readPPM*** -> citeste un fisier ppm si returneaza matricea de pixeli corespunzatoare*/
RGB** readPPM(char* fisier_intrare1, int* w, int* h){
	FILE* fileIn = fopen(fisier_intrare1, "r");
	char* str = malloc(10 * sizeof(char));
	unsigned int width = 0, height = 0;
	fscanf(fileIn, "%s", str);
	fscanf(fileIn, "%u%u", &width, &height);
	fscanf(fileIn, "%s", str);
	char c = 0;
	int i = 0, j = 0;
	fscanf(fileIn, "%c", &c);

	RGB** rgb = (RGB**)calloc(height, sizeof(RGB*));
	for(i = 0; i < height; i++)
		rgb[i] = (RGB*)calloc(width, sizeof(RGB*));

	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			fread(&rgb[i][j].red, sizeof(unsigned char), 1, fileIn);
			fread(&rgb[i][j].green, sizeof(unsigned char), 1, fileIn);
			fread(&rgb[i][j].blue, sizeof(unsigned char), 1, fileIn);
		}
	}
	*w = width;
	*h = height;

	fclose(fileIn);
	free(str);
	return rgb; 
}

/***FUNCTIA avgColor*** -> returneaza prin parametrii culorile medii ale unei matrice de pixeli*/
void avgColor(RGB** rgb,  unsigned int x, unsigned int y, unsigned int size, 
	unsigned char *avgRed, unsigned char *avgGreen, unsigned char *avgBlue){

	double red = 0, green = 0, blue = 0;
	int i = 0, j = 0;
	for(i = x; i < (x + size); i++){
		for(j = y; j < (y + size); j++){
			red += rgb[i][j].red;
			green += rgb[i][j].green;
			blue += rgb[i][j].blue;
		}
	}
	(*avgRed) = red / (size * size);
	(*avgGreen) = green / (size * size);
	(*avgBlue) = blue / (size * size);
}

/***FUNCTIA mean*** -> returneaza mean ul dupa formula corespunzatoare a unei matrice de pixeli*/
int64_t mean(RGB** rgb, unsigned int x, unsigned int y, unsigned int size){
	unsigned char red = 0, green = 0, blue = 0;
	int64_t sum = 0;
	int i = 0, j = 0;
	avgColor(rgb, x, y, size, &red, &green, &blue);
	for(i = x; i < (x + size); i++){
		for(j = y; j < (y + size); j++){
			sum += (red - rgb[i][j].red) * (red - rgb[i][j].red);
			sum += (green - rgb[i][j].green) * (green - rgb[i][j].green);
			sum += (blue - rgb[i][j].blue) * (blue - rgb[i][j].blue);
		}
	}
	return sum / (3 * size * size);
}

/***FUNCTIA newQnode*** -> creeaza un nou nod al unui quadtree*/
qnode* newQnode(unsigned char red, unsigned char green, unsigned char blue, uint32_t area){
	qnode* Q = (qnode*)calloc(1, sizeof(qnode));
	Q->blue = blue;
	Q->green = green;
	Q->red = red;
	Q->area = area;
	Q->top_left = NULL;
	Q->top_right = NULL;
	Q->bottom_right = NULL;
	Q->bottom_left = NULL;
	return Q;
}

/***FUNCTIA quadtree*** -> creeaza quadtree ul corespunzator unei matrice de pixeli prin 
	recursivitate*/
qnode* quadtree(RGB** rgb,unsigned int x, unsigned int y, unsigned int size, double factor){
	unsigned char red = 0, green = 0, blue = 0;
	avgColor(rgb, x, y, size, &red, &green, &blue);
	qnode* root = newQnode(red, green, blue, size * size);

	if(mean(rgb, x, y, size) > factor){
		size = size / 2;
		root->top_left = quadtree(rgb, x, y, size, factor);
		root->top_right = quadtree(rgb, x, y + size, size, factor);
		root->bottom_right = quadtree(rgb, x + size, y + size, size, factor);
		root->bottom_left = quadtree(rgb, x + size, y, size, factor);
	}else{
		avgColor(rgb, x, y, size, &red, &green, &blue);
		root->red = red;
		root->green = green;
		root->blue = blue;
	}

	return root;
}

/***FUNCTIA count*** -> returneaza nr de noduri ale unui quadtree*/
int count(qnode* root){
	if(root == NULL){
		return 0;
	}else{
		return 1 + count(root->top_left) + count(root->top_right) + count(root->bottom_right) + count(root->bottom_left);
	}
}

/***FUNCTIA countLeaves*** -> returneaza nr de frunze ale unui quadtree*/
uint32_t countLeaves(qnode* root){
	if(root == NULL)
		return 0;

	if(root->top_left == NULL && root->top_right == NULL && root->bottom_right == NULL && root->bottom_left == NULL)
		return 1;

	return countLeaves(root->top_left) + countLeaves(root->top_right) + countLeaves(root->bottom_right) + countLeaves(root->bottom_left);
}

/***FUNCTIA createQueue*** -> creeaza o coada de qnode* cu lungimea MAX_Q_SIZE*/
qnode** createQueue(int* front, int* rear){
	qnode** queue = (qnode**)calloc(MAX_Q_SIZE, sizeof(qnode*));

	*front = *rear = 0;
	return queue;
}

/***FUNCTIA enQueue*** -> adauga un element la coada*/
void enQueue(qnode** queue, int* rear, qnode* new_node){
	queue[*rear] = new_node;
	(*rear)++;
}

/***FUNCTIA deQueue*** -> scoate un element din coada*/
qnode* deQueue(qnode** queue, int* front){
	(*front)++;
	return queue[*front - 1];
}

/***FUNCTIA QuadtreeToArray*** -> parcurge pe nivele arborele mai intai pt a completa indecsi 
	corespunzatori pt vector pt fiecare element, iar apoi se parcurge din nou arborele in 
	acelasi fel pt a compplata vectorul de QuadtreeNode uri. Parcurgerea pe nivele(BFS) este 
	facuta folosind o coada*/
void QuadtreeToArray(qnode* root, QuadtreeNode* qArray){
	int rear = 0, front = 0, i = 0;
	qnode** queue1 = createQueue(&front, &rear);
	qnode* temp_node = root;

	while(temp_node != NULL) {
		temp_node->index = i;
		i++;

        /*Enqueue child1 */
		if (temp_node->top_left)
			enQueue(queue1, &rear, temp_node->top_left);

        /*Enqueue child2 */
		if (temp_node->top_right)
			enQueue(queue1, &rear, temp_node->top_right);

        /*Enqueue child3 */
		if (temp_node->bottom_right)
			enQueue(queue1, &rear, temp_node->bottom_right);

 		/*Enqueue child4 */
		if (temp_node->bottom_left)
			enQueue(queue1, &rear, temp_node->bottom_left);

        /*Dequeue node and make it temp_node*/
		temp_node = deQueue(queue1, &front);
	}
	free(queue1);	

	i = 0;
	qnode** queue2 = createQueue(&front, &rear);
	temp_node = root;
	while(temp_node != NULL) {
		qArray[i].red = temp_node->red;
		qArray[i].green = temp_node->green;
		qArray[i].blue = temp_node->blue;
		qArray[i].area = temp_node->area;

		if(temp_node->top_left == NULL)
			qArray[i].top_left = -1;
		else
			qArray[i].top_left = temp_node->top_left->index;

		if(temp_node->top_right == NULL)
			qArray[i].top_right = -1;
		else
			qArray[i].top_right = temp_node->top_right->index;

		if(temp_node->bottom_right == NULL)
			qArray[i].bottom_right = -1;
		else
			qArray[i].bottom_right = temp_node->bottom_right->index;

		if(temp_node->bottom_left == NULL)
			qArray[i].bottom_left = -1;
		else
			qArray[i].bottom_left = temp_node->bottom_left->index;

		i++;

		if (temp_node->top_left)
			enQueue(queue2, &rear, temp_node->top_left);

		if (temp_node->top_right)
			enQueue(queue2, &rear, temp_node->top_right);

		if (temp_node->bottom_right)
			enQueue(queue2, &rear, temp_node->bottom_right);

		if (temp_node->bottom_left)
			enQueue(queue2, &rear, temp_node->bottom_left);

		temp_node = deQueue(queue2, &front);
	}
	free(queue2);
}

/***FUNCTIA qArrayToBin*** -> scrie in fisierul binar 'fisier_iesire' vectorul qArray*/
void qArrayToBin(char* fisier_iesire, QuadtreeNode* qArray, uint32_t len, uint32_t colors){
	FILE* fileOut = fopen(fisier_iesire, "wb");

	fwrite(&colors, sizeof(uint32_t), 1, fileOut);
	fwrite(&len, sizeof(uint32_t), 1, fileOut);
	int i = 0;
	for (i = 0; i < len; ++i){
		fwrite(&qArray[i], sizeof(QuadtreeNode), 1, fileOut);
	}
	fclose(fileOut);
}

/***FUNCTIA compress*** -> compreseaza o imagine .ppm intr un fisier binar cu ajutorul unui 
	quadtree*/
void compress(double factor, char* fisier_intrare1, char* fisier_iesire){
	int width = 0, height = 0;
	RGB** rgb = readPPM(fisier_intrare1, &width, &height);

	//create quadtree
	qnode* root = quadtree(rgb, 0, 0, width, factor);

	//create array
	uint32_t len = count(root);
	QuadtreeNode* qArray = calloc(len, sizeof(QuadtreeNode));
	QuadtreeToArray(root, qArray);

	//array to binary file
	uint32_t colors = countLeaves(root);
	qArrayToBin(fisier_iesire, qArray, len, colors);

	freeMat(rgb, width);
	free(qArray);
	freeQTree(root);
}

/***FUNCTIA binToQArray*** -> citeste un fisier binar si completeaza qArray ul pe baza acestuia*/
QuadtreeNode* binToQArray(char* fisier_intrare1){
	FILE* fileIn = fopen(fisier_intrare1, "rb");
	uint32_t colors = 0, size = 0;
	fread(&colors, sizeof(uint32_t), 1, fileIn);
	fread(&size, sizeof(uint32_t), 1, fileIn);

	QuadtreeNode* qArray = calloc(size, sizeof(QuadtreeNode));
	int i = 0;
	for(i = 0; i < size; i++){
		fread(&qArray[i], sizeof(QuadtreeNode), 1, fileIn);
	}

	fclose(fileIn);
	return qArray;
}

/***FUNCTIA qArrayToQuadtree*** -> creeaza in mod recursiv un quadtree pe baza qArray ului*/
qnode* qArrayToQuadtree(QuadtreeNode* qArray, uint32_t index){
	qnode* root = newQnode(qArray[index].red, qArray[index].green, qArray[index].blue, qArray[index].area);

	if(qArray[index].top_left > 0)
		root->top_left = qArrayToQuadtree(qArray, qArray[index].top_left);

	if(qArray[index].top_right > 0)
		root->top_right = qArrayToQuadtree(qArray, qArray[index].top_right);

	if(qArray[index].bottom_right > 0)
		root->bottom_right = qArrayToQuadtree(qArray, qArray[index].bottom_right);

	if(qArray[index].bottom_left > 0)
		root->bottom_left = qArrayToQuadtree(qArray, qArray[index].bottom_left);

	return root;
}

/***FUNCTIA fill*** -> umple o matrice de pixeli cu, culoarea data ca parametru*/
void fill(RGB** rgb, unsigned int x, unsigned int y, uint32_t size, 
	unsigned char red, unsigned char green, unsigned char blue){
	int i = 0, j = 0;
	for(i = x; i < (x + size); i++){
		for(j = y; j < (y + size); j++){
			rgb[i][j].red = red;
			rgb[i][j].green = green;
			rgb[i][j].blue = blue;		
		}
	}
}

/***FUNCTIA QuadtreeToMat*** -> completeaza matricea de pixeli conform quadtree ului dat ca 
	parametru in mod recursiv*/
void QuadtreeToMat(qnode* root, RGB** rgb, unsigned int x, unsigned int y, uint32_t size){
	fill(rgb, x, y, size, root->red, root->green, root->blue);
	size = size / 2;

	if(root->top_left)
		QuadtreeToMat(root->top_left, rgb, x, y, size);

	if(root->top_right)
		QuadtreeToMat(root->top_right, rgb, x, y + size, size);

	if(root->bottom_right)
		QuadtreeToMat(root->bottom_right, rgb, x + size, y + size, size);

	if(root->bottom_left)
		QuadtreeToMat(root->bottom_left, rgb, x + size, y, size);
}

/***FUNCTIA writePPM*** -> creeaza fisierul .PPM pe baza matricei data ca parametru*/
void writePPM(char* fisier_iesire, RGB** rgb, uint32_t size){
	FILE* fileOut = fopen(fisier_iesire, "w");
	fprintf(fileOut, "P6\n");
	fprintf(fileOut, "%d %d\n", size, size);
	fprintf(fileOut, "%d\n", 255);

	int i = 0, j = 0;
	for(i = 0; i < size; i++){
		for(j = 0; j < size; j++){
			fwrite(&rgb[i][j].red, sizeof(unsigned char), 1, fileOut);
			fwrite(&rgb[i][j].green, sizeof(unsigned char), 1, fileOut);
			fwrite(&rgb[i][j].blue, sizeof(unsigned char), 1, fileOut);
		}
	}
	fclose(fileOut);
}

/***FUNCTIA decompress*** -> decompreseaza un fisier binar intr o imagine .ppm*/
void decompress(char* fisier_intrare1, char* fisier_iesire){
	//din fisier bin(intrare) -> qArray ->quadtree ->matrix ->imag.ppm
	QuadtreeNode* qArray = binToQArray(fisier_intrare1);
	qnode* root = qArrayToQuadtree(qArray, 0);

	uint32_t size = sqrt(root->area);
	RGB** rgb = (RGB**)calloc(size, sizeof(RGB*));
	int i = 0;
	for(i = 0; i < size; i++)
		rgb[i] = (RGB*)calloc(size, sizeof(RGB*));

	QuadtreeToMat(root, rgb, 0, 0, size);
	writePPM(fisier_iesire, rgb, size);

	freeMat(rgb, size);
	free(qArray);
	freeQTree(root);
}

/***FUNCTIA swap*** -> interschimba 2 noduri ale arborelui*/
void swap(qnode** a, qnode** b){
	qnode* aux = *a;
	*a = *b;
	*b = aux;
}

/***FUNCTIA invert*** -> inverseaza un quadtree recursiv in fct de axa data ca parametru*/
void invert(qnode* root, int vert){
	if(root == NULL){
		return;

	}else{

		invert(root->top_left, vert);
		invert(root->top_right, vert);
		invert(root->bottom_right, vert);
		invert(root->bottom_left, vert);
		
		if(!vert){
			swap(&root->top_left, &root->bottom_left);
			swap(&root->top_right, &root->bottom_right);
		}else{
			swap(&root->top_left, &root->top_right);
			swap(&root->bottom_left, &root->bottom_right);
		}
	}
}

/***FUNCTIA mirror*** -> inverseaza o imagine pe axa data ca parametru utilizand un quadtree*/
void mirror(char* type, double factor, char* fisier_intrare1, char* fisier_iesire){
	//readfile->matrix->tree->mirror tree
	int width = 0, height = 0;
	RGB** rgb = readPPM(fisier_intrare1, &width, &height);
	qnode* root = quadtree(rgb, 0, 0, width, factor);

	if(strcmp(type, "h"))
		invert(root, 0);
	else
		invert(root, 1);

	QuadtreeToMat(root, rgb, 0, 0, width);
	writePPM(fisier_iesire, rgb, width);

	freeMat(rgb, width);
	freeQTree(root);
}

int main(int argc, char const *argv[]){

	char* fisier_iesire;
	char* fisier_intrare1;
	fisier_intrare1 = (char*)malloc(100 * sizeof(char));
	fisier_iesire = (char*)malloc(100 * sizeof(char));

	strcpy(fisier_intrare1, argv[argc - 2]);
	strcpy(fisier_iesire, argv[argc - 1]);

	if(strcmp("-c", argv[1]) == 0){
		//COMPRESIE IMAGINE
		double factor;
		factor = atoi(argv[2]);
		compress(factor, fisier_intrare1, fisier_iesire);

	}else if(strcmp("-d", argv[1]) == 0){
		//DECOMPRESIE IMAGINE
		decompress(fisier_intrare1, fisier_iesire);

	}else if(strcmp("-m", argv[1]) == 0){
		//MIRROR IMAGINE	
		char* type = malloc(2 * sizeof(char));
		strcpy(type, argv[2]);
		double factor = atoi(argv[3]);
		mirror(type, factor, fisier_intrare1, fisier_iesire);
		free(type);
	}

	free(fisier_iesire);
	free(fisier_intrare1);
	return 0;
}
