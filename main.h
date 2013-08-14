/*
 * main.h
 *
 *  Created on: Sep 25, 2012
 *      Author: bpassuello
 */

#ifndef MAIN_H_
#define MAIN_H_

/**
 * \mainpage Quadtree Program
 * \brief This program reads in data from a raster image and
 *         creates a minimal quadtree by collapsing homogeneous quadrants.
 * \file main.cpp
 * \brief This program implements a quadtree that collapses homogeneous quadrants.
 * \details This program does some stuff.
 * \date Jul 10, 2012
 * \update October 25, 2012
 * \author Brian Passuello
 */

#include "Utility.h"
#include "Quadtree.h"

using namespace std;

/**
 * \brief This will ask the user to input the necessary values.
 */
void initializeC();

/**
 * \brief Read in a GDAL formatted image.
 */
float** readImageC(GDALDatasetH);

/**
 * \brief Creates a GDAL supported file based on a given 2D array.
 */
QT_ERR createImageC(GDALDatasetH, float**, int, int, string);

/**
 * \brief Dumps contents of data to a file.
 */
QT_ERR dumpToFile(float**,int, int, string);

/**
 * \brief Prints information about the dataset.
 */
void printGDALDatasetInfo(GDALDataset*);

/**
 * \brief Prints information about a GDALRasterBand.
 */
void printGDALRasterInfo(GDALRasterBand*);

/**
 * \brief This function reads the file that holds the output data
 *             and recreates the original image using the data.
 */
float** readNodeFile(string);

/**
 * \brief A typical implementation of quicksort.
 */
void sortNodes(Node[],int, int);

/**
 * \brief This will create a DOT file.
 */
QT_ERR DrawTree(Quadtree*);


#endif /* MAIN_H_ */
