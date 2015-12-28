#include <string>
#include <vector>

struct Image;
void image_chat_ocr(Image *s);
int image_troop_count(Image *s, const char *filename);
int image_base_count(Image *s, const char *filename);
std::vector<int> image_find_red_line(Image *s);
