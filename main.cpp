#include "main.h"
//#include "Windows.h"

//string			inimgfn = "Images\/denvermsa.img";
string 			inimgfn = "Images\/downtown.img";
//string          inimgfn = "Images\\colorado_nlcd.img";

string 			outimgfn = "Results\/reclassified_image1.img";
GDALColorTable*	colorTable;
Quadtree*     	original_tree;
Quadtree*    	reclass_tree;
float**     	raster_data;
int         	xsize = 0;
int         	ysize = 0;

GDALDatasetH hDataset;

double times[] = {0,0,0,0};

double stoptime(void) {
   struct timeval t;
   gettimeofday(&t,NULL);
   return (double) t.tv_sec + t.tv_usec/1000000.0;
}
 
QT_ERR dumpToFile(float** img,int w, int h, string fname){
    ofstream out;
    out.open(fname.c_str(),ios::out);
    if(out.fail()) return (QT_ERR)WRITE_ERROR;

    for(int i=0;i<w;i++){
        for(int j=0;j<h;j++){
            out << img[i][j] << " ";
        }
        out << endl;
    }

    return (QT_ERR)NO_ERROR;
}

/**
 * \brief This is the main function that creates a quadtree.
 */
int main(int argc, char* argv[]){

	/*
    cout << "Size of int: " << sizeof(int) << endl;
    cout << "Size of float: " << sizeof(float) << endl;
    long i = 291000000;
    while(true){
        int* test = new int[i];
        cerr << i << endl;
        free(test);
        i+=100000;
    }
    */
    
    if(argc == 2){
        inimgfn = argv[1];
    }
    else if(argc > 2){
        cerr << "Usage: " << argv[0] << " FILENAME" << endl;
        exit(1);
    }
    
    cerr << "Filename: " << inimgfn << endl;

    initializeC();
    
    double q = stoptime();
    
	raster_data = readImageC(hDataset);
    times[0] = stoptime() - q;

    cerr << "Image read\n";
    
    assert(raster_data!=NULL);

    q = stoptime();
	original_tree = Quadtree::constructTree(raster_data,xsize,ysize);
    q = stoptime() - q;
    
    cerr << "Tree constructed\n";
	times[1] = q;
	
    assert(original_tree!=NULL);

    cerr << "passed assert\n";
    q = stoptime();
	original_tree->Prune();
	cerr << "pruned\n";
    times[2] = stoptime() - q;
    
    float** newimg2 = original_tree->RebuildImage();
    
    q = stoptime();
	createImageC(hDataset,newimg2,xsize,ysize,outimgfn);
    q = stoptime() - q;

    cerr << "output\n";
	times[3]+= q;

    ofstream out;
    out.open("QuadtreeResults.txt",ios::app);

    out << "Reading: " << times[0] << endl <<
            "Construction: " << times[1] << endl <<
            "Reclass/Pruning: " << times[2] << endl <<
            "Output: " << times[3] << endl << endl;
            
    cerr << "Reading: " << times[0] << endl <<
            "Construction: " << times[1] << endl <<
            "Reclass/Pruning: " << times[2] << endl <<
            "Output: " << times[3] << endl << endl;
	return 1;
}

/***********************************************************
 *
 *
 * FUNCTIONS
 *
 *
 ***********************************************************/


void initializeC(){
    GDALAllRegister();

    hDataset = GDALOpen(inimgfn.c_str(),GA_ReadOnly);
    if(!hDataset){
    	cerr << "Error opening hDataset\n";
    	exit(1);
    }
    xsize = GDALGetRasterXSize(hDataset);
    ysize = GDALGetRasterYSize(hDataset);
}

/*
 * X Y Width Height Value Level
 */
float** readNodeFile(string filename){
	fprintf(stderr,"Reading node file: %s\n",filename.c_str());

    ifstream in;
    in.open(filename.c_str());
    if(in.fail()){
    	fprintf(stderr, "Failed to open input file\"%s\"\n",filename.c_str());
        return NULL;
    }

    string r;
    int i,j,x,y,numNodes,xsizelocal,ysizelocal;

    getline(in,r);
    istringstream row(r);
    if(r!="NODEFILE"){
        cerr << "Error: File \"" << filename << "\" is not a nodefile.\n";
        cerr << "Got \"" << r << "\", Expected \"NODEFILE\"\n";
        return NULL;
    }

    getline(in,r);
    istringstream row2(r);
    row2 >> numNodes >> xsizelocal >> ysizelocal;

    fprintf(stderr,"Nodes: %d X: %d Y: %d\n",numNodes,xsizelocal,ysizelocal);

    fprintf(stderr,"Allocating array\n");

    Node nodeArr[numNodes];

    fprintf(stderr,"Made the node array\n");
    int** nodes = new int*[numNodes];
    for(i=0;i<numNodes;i++)
        nodes[i]=new int[5];
    fprintf(stderr,"Made the 2d array\n");

    fprintf(stderr,"Completed allocating the img array\n");

    float** img = new float*[xsizelocal];
    for(i=0;i<xsizelocal;i++)
        img[i]=new float[ysizelocal];

    fprintf(stderr,"About to enter for loop\n");
    for(i=0;i<numNodes;i++)
    {
        getline(in,r);
        stringstream row(r);
        for(j=0;j<5;j++)
            row >> nodes[i][j];

        Node a = {nodes[i][0],nodes[i][1],nodes[i][2],
                nodes[i][3],nodes[i][4],nodes[i][5]};
        nodeArr[i] = a;

        for(x=nodes[i][0];x<nodes[i][0]+nodes[i][2];x++)
            for(y=nodes[i][1];y<nodes[i][1]+nodes[i][3];y++)
                img[x][y]=nodes[i][4];
    }

    for(i=0;i<numNodes;i++) delete [] nodes[i];
    delete [] nodes;

    return img;

}

float** readImageC(GDALDatasetH gdalData){

    try{
        int i,j,x,y;
        float *pafScanline;
        float** myData;

        cerr << "Getting bands\n";
        GDALRasterBandH gdalBand = GDALGetRasterBand(gdalData,1);

        if(gdalBand == NULL){
            cerr << "error null band\n"; exit(1);
        }
        
        cerr << "Getting no data value\n";
        emptyValue = GDALGetRasterNoDataValue(gdalBand, NULL);

        x = GDALGetRasterBandXSize(gdalBand);
        y = GDALGetRasterBandYSize(gdalBand);

        
        cerr << "x,y " << x << "," << y << endl;
        myData = new float*[x];
        for(i=0;i<x;i++) myData[i] = new float[y];

        cerr << "myData finished\n";
        cerr << "x*y = " << x*y << endl;
        cerr << "Size of float: " << sizeof(float) << ", " << sizeof(float)*x*y << endl;
        //byte* test = new byte[x*y];
        cerr << "bytes worked\n";
        pafScanline = new float[x*y];
        cerr << "scanline finished\n";
        for(i=0;i<x;i++)
            for(j=0;j<y;j++)
                myData[i][j] = emptyValue;

        cerr << "IO\n";
        GDALRasterIO(gdalBand,GF_Read,0,0,x,y,
                pafScanline,x,y,GDT_Float32,0,0);

        for(j=0;j<y;++j){
            for(int k=0;k<x;++k){
                myData[k][j] = pafScanline[j*x+k];
            }
        }

        delete [] pafScanline;

        return myData;
    }
    catch(bad_alloc& ba){
        cerr << ba.what() << endl;
        exit(1);
    }
}

QT_ERR createImageC(GDALDatasetH poSrcDS, float** data, int w, int h, string fname){

	//fprintf(stderr,"Creating GDAL image %s\n",fname.c_str());
	GDALDriverH gdalDriver = GDALGetDriverByName(outformat);
    if( gdalDriver == NULL ) return (QT_ERR)READ_ERROR;

    char **gdalMetadata = GDALGetMetadata( gdalDriver , NULL );

    if( !CSLFetchBoolean( gdalMetadata, GDAL_DCAP_CREATECOPY, FALSE ) ){
        printf( "Driver %s does not support CreateCopy() method."
                "\nCannot create image.\n", outformat );
        return (QT_ERR)WRITE_ERROR;
    }

    GDALDatasetH poDstDS = GDALCreateCopy(gdalDriver,
    									fname.c_str(),
    									poSrcDS,
    									FALSE,
    									NULL,
    									NULL,
    									NULL
    									);

    GDALRasterBandH band = GDALGetRasterBand(poDstDS,1);

    if(band == NULL){
        cerr << "NULL BAND\n";
        return (QT_ERR)NO_ERROR;
    }
    
    for (int i=0; i<w; i++) {
        //cerr << "Writing band " << i << " of " << w << endl;
        GDALRasterIO(band,
                     GF_Write,
                     i,0,
                     1,h,
                     data[i],
                     1,h,
                     GDT_Float32,
                     0,0);
    }

    GDALClose( poDstDS );

    return (QT_ERR)NO_ERROR;
}

int Compact1By1(int x)
{
  x &= 0x55555555;                  // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
  x = (x ^ (x >>  1)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
  x = (x ^ (x >>  2)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
  x = (x ^ (x >>  4)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
  x = (x ^ (x >>  8)) & 0x0000ffff; // x = ---- ---- ---- ---- fedc ba98 7654 3210
  return x;
}

int DecodeMorton2X(int code)
{
  return Compact1By1(code >> 0);
}

int DecodeMorton2Y(int code)
{
  return Compact1By1(code >> 1);
}

int Part1By1(int x)
{
  x &= 0x0000ffff;                  // x = ---- ---- ---- ---- fedc ba98 7654 3210
  x = (x ^ (x <<  8)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
  x = (x ^ (x <<  4)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
  x = (x ^ (x <<  2)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
  x = (x ^ (x <<  1)) & 0x55555555; // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
  return x;
}

int EncodeMorton2(int x, int y)
{
  return (Part1By1(y) << 1) + Part1By1(x);
}

///TODO: Fix
QT_ERR DrawTree(Quadtree* tree){

    ofstream out;
    out.open("graph.dot",ios::out);

    if(out.fail()){
        return (QT_ERR)WRITE_ERROR;
    }
    out << "graph reclassified_tree.gv {" << endl;
    if(tree->DrawTree(out) != NO_ERROR){
        cerr << "Error drawing tree\n";
        return (QT_ERR)FILE_ERROR;
    }
    out << "}";
    out.close();

    return (QT_ERR)NO_ERROR;

}
