#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <png.h>

#include "disp.h"
#include "prog.h"
#include "util.h"
#include "col.h"
#include "layout.h"

enum Constants {
	scr_MAX_FILENAME = 256
};
static GLubyte* pixels = NULL;
static GLuint fbo;
static GLuint rbo_color;
static GLuint rbo_depth;
static int offscreen = 1;
static unsigned int max_nframes = 128;
static unsigned int nframes = 0;
static unsigned int time0;
static unsigned int height = 128;
static unsigned int width = 128;
#define PPM_BIT (1 << 0)
#define LIBPNG_BIT (1 << 1)
#define FFMPEG_BIT (1 << 2)
static unsigned int output_formats = PPM_BIT | LIBPNG_BIT | FFMPEG_BIT;

/* Adapted from https://github.com/cirosantilli/cpp-cheat/blob/19044698f91fefa9cb75328c44f7a487d336b541/png/open_manipulate_write.c */
static png_byte *png_bytes = NULL;
static png_byte **png_rows = NULL;
static void scrPng(const char *filename, unsigned int width, unsigned int height, GLubyte **pixels, png_byte **png_bytes, png_byte ***png_rows) {
	size_t i, nvals;
	const size_t format_nchannels = 4;
	FILE *f = fopen(filename, "wb");
	nvals = format_nchannels * width * height;
	*pixels = (GLubyte*) realloc(*pixels, nvals * sizeof(GLubyte));
	*png_bytes = (png_byte*) realloc(*png_bytes, nvals * sizeof(png_byte));
	*png_rows = (png_byte**) realloc(*png_rows, height * sizeof(png_byte*));
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, *pixels);
	for (i = 0; i < nvals; i++)
		(*png_bytes)[i] = (*pixels)[i];
	for (i = 0; i < height; i++)
		(*png_rows)[height - i - 1] = &(*png_bytes)[i * width * format_nchannels];
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) abort();
	png_infop info = png_create_info_struct(png);
	if (!info) abort();
	if (setjmp(png_jmpbuf(png))) abort();
	png_init_io(png, f);
	png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);
	png_write_image(png, *png_rows);
	png_write_end(png, NULL);
	png_destroy_write_struct(&png, &info);
	fclose(f);
}

int main(int argc, char* argv[]) {
	Disp disp("asdf", 160, 171);

	if (argc != 2) {
		std::cout << "Error: Wrong number of arguments" << std::endl;

		return 1;
	}

	if (strlen(argv[1]) > 1) {
		std::cout << "Error: Length of argument longer than one" << std::endl;

		return 1;
	}

	char c = argv[1][0];

	// data
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// position
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	std::vector<GLfloat> vtc = util::rdAttr(std::string(1, c), 0);
	glBufferData(GL_ARRAY_BUFFER, vtc.size() * sizeof (GLfloat), &vtc[0], GL_STATIC_DRAW);

	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	std::vector<GLushort> idc = util::rdIdc(std::string(1, c), 0);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, idc.size() * sizeof (GLushort), &idc[0], GL_STATIC_DRAW);

	// matrix
	const GLfloat scaleFac = 0.8;

	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, glm::vec3(-1.0, 1.0, 0.0));
	model = glm::translate(model, glm::vec3(margin * 2, -(margin * 2), 0.0));
	model = glm::scale(model, glm::vec3(scaleFac, scaleFac, scaleFac));

	// shader
	Prog prog("shad", "shad");

	/// attribute
	GLint attrPos = glGetAttribLocation(prog._id, "pos");
	glVertexAttribPointer(attrPos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) 0);
	glEnableVertexAttribArray(attrPos);

	/// uniform
	GLint uniModel = glGetUniformLocation(prog._id, "model");

	// initialize
	prog.use();

	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

	disp.clear(col[true].r / 255.0, col[true].g / 255.0, col[true].b / 255.0, 1);

	glDrawElements(GL_TRIANGLES, idc.size(), GL_UNSIGNED_SHORT, (GLvoid*) 0);

	disp.update();

	GLubyte* pixels = NULL;
	scrPng(std::string(std::string("o/") + std::string(1, c) + ".png").c_str(), 160, 171, &pixels, &png_bytes, &png_rows);
}
