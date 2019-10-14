//#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <numpy/arrayobject.h>

// Define functions below

static PyObject *load_frames_legacy(PyObject *self, PyObject *args);

/////// Python-module-related functions and tables

// The module's method table
static PyMethodDef _legacy_cMethods[] = {
    {"load_frames_legacy", load_frames_legacy, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL}
};

// The module definition function
static struct PyModuleDef _legacy_c = {
    PyModuleDef_HEAD_INIT,
    "_legacy_c",
    NULL, // Module documentation
    -1,
    _legacy_cMethods
};

// The module initialization function
PyMODINIT_FUNC PyInit__legacy_c(void) { 
        import_array(); //Numpy
        return PyModule_Create(&_legacy_c);
};

static PyObject *load_frames_legacy(PyObject *self, PyObject *args) {

    int32_t startFrame, frameN, rowSize, rowN;
    const char *fname;
    PyObject *frames_o;
    
    if(!PyArg_ParseTuple(args, "siiiiO", 
        &fname, &startFrame, &frameN, &rowSize, &rowN, &frames_o)) return NULL;
    
    PyObject *frames_a = PyArray_FROM_OTF(frames_o, NPY_UINT16, NPY_IN_ARRAY);
        
    // Check that the above conversion worked, otherwise decrease the reference
    // count and return NULL.                                 
    if (frames_a == NULL) {
        Py_XDECREF(frames_a);
        return NULL;
    }
    
    // Get pointers to the data in the numpy arrays.
    uint16_t *frames = (uint16_t*)PyArray_DATA(frames_a);
    
    //////////////////////////////////
    //////////////////////////////////
    //////////////////////////////////
    //////////////////////////////////
    // Actual C code
    //////////////////////////////////
    //////////////////////////////////
    //////////////////////////////////
    //////////////////////////////////
    
    std::ifstream file(fname, std::ios::in|std::ios::binary);
    
    // file.read() uses a pointer to char as destination. Declare a pointer to
    // char that will point to the array of uint16_t frames, and calculate the
    // sizes of rows and frames in chars (and not pixels, which are uint16_t).
    char *memblock_r, *memblock_g;
    int32_t channelsize_char = rowSize*rowN*2;
    int32_t rowsize_char = rowSize*2;
    
    //int64_t initialPosition = startFrame*2*channelsize_char;
    //file.seekg((std::ios::streampos)initialPosition, std::ios::beg);
    
    // This is weird, but I haven't managed to understand how to correctly
    // support large files.
    for(int j=0;j<startFrame;j++){
        file.seekg(2*channelsize_char,std::ios::cur);
    }
    
    // Convert the pointer to the frames array from a pointer to uint16_t to 
    // a pointer to char, so that it can be correctly used in file.read().
    memblock_r = (char*)frames;
    memblock_g = (char*)frames + channelsize_char;
    
    int32_t frameidx;
    // For each frame
    for(int i=0;i<frameN;i++){
        frameidx = i + startFrame;
        // For each full row
        for(int n=0;n<rowN;n++){
            // Copy the first half of the row (red).
            // Move absolute: this does not work because the numbers become
            // too large for in32_t!
            //memblock = (char*)frames + 2*i*channelsize_char + n*rowsize_char;
            //file.seekg(2*frameidx*channelsize_char+2*n*rowsize_char, std::ios::beg);
            file.read(memblock_r, rowsize_char);
            // Move the pointer a frame (i.e. channel, half of the full frame
            // for each time) forward.
            // Move absolute: this does not work because the numbers become
            // too large for in32_t!
            //memblock = (char*)frames + (2*i+1)*channelsize_char + n*rowsize_char;
            //file.seekg(2*frameidx*channelsize_char+(2*n+1)*rowsize_char, std::ios::beg);
            // Read the second half row (green).
            file.read(memblock_g, rowsize_char);
            // If you're not at the end of the allocated frames, move one full
            // row forward.
            if(!(i==(frameN-1) && n==(rowN-1))) {
                memblock_r = memblock_r + rowsize_char;
                memblock_g = memblock_g + rowsize_char;
            }
        }
        if(i<(frameN-1)){
            memblock_r = memblock_r + channelsize_char;
            memblock_g = memblock_g + channelsize_char;
        }
    }
    
    file.close();
    
    //////////////////////////////////
    //////////////////////////////////
    //////////////////////////////////
    //////////////////////////////////
    // End of C code
    //////////////////////////////////
    //////////////////////////////////
    //////////////////////////////////
    //////////////////////////////////
    
    
    // Decrease the reference count for the python objects that have been 
    // declared in this function.
    Py_XDECREF(frames_a);
    
    // Return the python object none. Its reference count has to be increased.
    Py_INCREF(Py_None);
    return Py_None;
}
