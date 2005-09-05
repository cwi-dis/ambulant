//#define	WITH_HTML_WIDGET // Project-><project>Properites->C/C++->Preprocessor->PreprocessorDefinitions
						// to enable/disable html renderer, for <project>=AmbulantPlayer,libambulant_win32.
#ifdef	JUNK
#include <string>

void* create_html_widget(std::string url, int left, int top, int width, int height);
int delete_html_widget(void* ptr);
void redraw_html_widget(void* ptr);
#endif // JUNK

#ifdef	WITH_HTML_WIDGET

#include <string>

class html_browser {
public:
	html_browser(int left, int top, int width, int height);
	~html_browser();
	void goto_url(std::string url);
	void show();
	void hide();

private:
	void* m_browser;
};
#endif // WITH_HTML_WIDGET
