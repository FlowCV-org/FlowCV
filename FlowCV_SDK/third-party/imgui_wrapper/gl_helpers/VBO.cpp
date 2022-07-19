#include"VBO.h"


// Binds the VBO
void VBO::Bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}

// Unbinds the VBO
void VBO::Unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Deletes the VBO
void VBO::Delete()
{
	glDeleteBuffers(1, &ID);
}

void VBO::Set(GLfloat *vertices, GLsizeiptr size, bool dynamic_draw)
{
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    if (!dynamic_draw)
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    else
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_DYNAMIC_DRAW);
}
