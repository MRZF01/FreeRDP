#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string>

#define BLOCK_SIZE 8192
#define LINEWIDTH 80

static void usage(const char* prg)
{
	fprintf(stderr, "%s <font file> <buffer file without extension> <buffer name>\n", prg);
}

static int write_header(FILE* out, const char* outname, const char* buffername, const char* font,
                        size_t size)
{
	std::string header = outname;
	auto pos = header.find_last_of("/");
	if (pos != std::string::npos)
		header = header.substr(pos + 1);
	pos = header.find_last_of("\\");
	if (pos != std::string::npos)
		header = header.substr(pos + 1);
	pos = header.find_last_of(".");
	if (pos != std::string::npos)
		header = header.substr(0, pos);
	fprintf(out, "/* AUTOGENERATED file, do not edit\n");
	fprintf(out, " *\n");
	fprintf(out, " * contains the converted font %s\n", font);
	fprintf(out, " */\n");
	fprintf(out, "#include <vector>\n");
	fprintf(out, "#include \"%s.hpp\"\n", header.c_str());
	fprintf(out, "\n");
	fprintf(out, "static std::vector<unsigned char> init();\n");
	fprintf(out, "const std::vector<unsigned char> %s = init();\n\n", buffername);
	fprintf(out, "std::vector<unsigned char> init() {\n");
	fprintf(out, "static const unsigned char data[] = {\n");

	return 0;
}

static int read(FILE* out, const char* font, const char* outname, const char* buffername)
{
	FILE* fp = fopen(font, "rb");
	if (!fp)
	{
		fprintf(stderr, "Failed to open font file '%s'\n", font);
		return -11;
	}

	fseek(fp, 0, SEEK_END);
	off_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	const int rc = write_header(out, outname, buffername, font, static_cast<size_t>(size));
	if (rc != 0)
	{
		fclose(fp);
		return -12;
	}

	int first = 1;
	while (!feof(fp))
	{
		char buffer[BLOCK_SIZE] = {};
		const size_t read = fread(buffer, 1, sizeof(buffer), fp);
		for (size_t x = 0; x < read; x++)
		{
			if (!first)
				fprintf(out, ",");
			first = 0;
			fprintf(out, "0x%02x", buffer[x] & 0xFF);
			if ((x % (LINEWIDTH / 5)) == 0)
				fprintf(out, "\n");
		}
	}

	fclose(fp);
	return rc;
}

static int write_trailer(FILE* out)
{
	fprintf(out, "};\n");
	fprintf(out, "return std::vector<unsigned char>(data, data + sizeof(data));\n");
	fprintf(out, "};\n");
	return 0;
}

int main(int argc, char* argv[])
{
	int rc = -3;
	if (argc != 4)
	{
		usage(argv[0]);
		return -1;
	}

	const char* font = argv[1];
	std::string header = argv[2];
	const char* name = argv[3];

	header += ".cpp";

	FILE* fp = fopen(header.c_str(), "w");
	if (!fp)
	{
		fprintf(stderr, "Failed to open header file '%s'", header.c_str());
		return -2;
	}

	rc = read(fp, font, header.c_str(), name);
	if (rc != 0)
		goto fail;
	rc = write_trailer(fp);

fail:
	fclose(fp);
	return rc;
}
