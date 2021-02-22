/**
 * @file VerticesFactory.cpp
 * 
 * @ingroup Simulator/Vertices
 *
 * @brief A factory class for creating Vertices objects.
 */

#include "VerticiesFactory.h"

#include "AllLIFNeurons.h"
#include "AllIZHNeurons.h"

/// Constructor is private to keep a singleton instance of this class.
VerticesFactory::VerticesFactory() {
   // register vertices classes
   registerClass("AllLIFNeurons", &AllLIFNeurons::Create);
   registerClass("AllIZHNeurons", &AllIZHNeurons::Create);
}

VerticesFactory::~VerticesFactory() {
   createFunctions.clear();
}

///  Register vertices class and its creation function to the factory.
///
///  @param  className  vertices class name.
///  @param  Pointer to the class creation function.
void VerticesFactory::registerClass(const string &className, CreateFunction function) {
   createFunctions[className] = function;
}


/// Creates concrete instance of the desired vertices class.
shared_ptr<IAllVertices> VerticesFactory::createVertices(const string &className) {
   verticesInstance = shared_ptr<IAllVertices>(invokeCreateFunction(className));
   return verticesInstance;
}

/// Create an instance of the vertices class using the static ::Create() method.
///
/// The calling method uses this retrieval mechanism in 
/// value assignment.
IAllVertices *VerticesFactory::invokeCreateFunction(const string &className) {
   for (auto i = createFunctions.begin(); i != createFunctions.end(); ++i) {
      if (className == i->first)
         return i->second();
   }
   return NULL;
}