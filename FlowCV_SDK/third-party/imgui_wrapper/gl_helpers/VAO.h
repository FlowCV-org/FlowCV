#ifndef VAO_CLASS_H
#define VAO_CLASS_H

#include <memory>
#include<glad/glad.h>
#include"VBO.h"

class VAO
{
public:
	// ID reference for the Vertex Array Object
	GLuint ID;

    void Set();
	// Links a VBO Attribute such as a position or color to the VAO
	void LinkAttrib(std::shared_ptr<VBO> vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
	// Binds the VAO
	void Bind();
	// Unbinds the VAO
	void Unbind();
	// Deletes the VAO
	void Delete();
};

#endif