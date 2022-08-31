#include <iostream>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <fstream>
#include <vector>

using namespace std;
using namespace sf;
using namespace tgui;

#define WORD_TOP_MARGIN 60
#define WORD_RIGHT_MARGIN 2
#define WORD_TEXT_SIZE 38

#define SECONDS 60
#define AVERAGE_WORD_LENGTH 5

string speed_matric = "WPM";

int charCount = 0;
int charError = 0;
int wordCount = 0;
int errorCount = 0;

int resX = VideoMode::getDesktopMode().width;
int resY = VideoMode::getDesktopMode().height;

RenderWindow window(VideoMode(resX, resY), "Typing", Style::Fullscreen);
GuiSFML gui{ window };

Panel::Ptr wordsbox;
Panel::Ptr metricPanel;
BitmapButton::Ptr refreshBtn;
Label::Ptr timelabel;
EditBox::Ptr typebox;
Label::Ptr accuracylabel;
Label::Ptr speedlabel;
Picture::Ptr loadingPic;
Panel::Ptr inputPanel;
Label::Ptr errorlabel;

float round2(float);
void reset();
void timeHide();
void timeEnd();
void resize();
void process();
void speedMatricChange();
int myrandom(int);

Timer::Ptr timer = Timer::create(&timeEnd, 60000, false);

int main()
{
    srand(time(NULL));
    
    gui.loadWidgetsFromFile("form.txt");
    gui.onViewChange(&resize);

    wordsbox = gui.get<Panel>("wordsbox");
    metricPanel = gui.get<Panel>("metricPanel");
    refreshBtn = gui.get<BitmapButton>("refreshBtn");
    timelabel = gui.get<Label>("time");
    typebox = gui.get<EditBox>("typebox");
    accuracylabel = gui.get<Label>("accuracy");
    speedlabel = gui.get<Label>("speed");
    loadingPic = gui.get<Picture>("loading");
    inputPanel = gui.get<Panel>("inputPanel");
    errorlabel = gui.get<Label>("errors");

    tgui::Font font = tgui::Font("times new roman.ttf");
    
    wordsbox->getRenderer()->setBorderColor(tgui::Color("#264d6e"));
    wordsbox->getRenderer()->setFont(font);

    metricPanel->getRenderer()->setBorderColor(tgui::Color("#264d6e"));
    
    refreshBtn->onClick(&reset);

    timelabel->onClick(&timeHide);
    
    speedlabel->onClick(&speedMatricChange);

    typebox->onTextChange(&process);
    typebox->getRenderer()->setFont(font);

    reset();
    
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            gui.handleEvent(event);

            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Escape)
                {
                    window.close();
                }
                else if (event.key.code == Keyboard::F5)
                {
                    reset();
                }
            }
        }

        if (timer->getNextScheduledTime())
        {
            int time = timer->getNextScheduledTime()->asSeconds();

            timelabel->setText(tgui::String(time));

            if (time != 0)
            {
                float speed = 0;

                if (speed_matric == "WPM")
                {
                    speed = ((charCount / AVERAGE_WORD_LENGTH) - errorCount) / ((float(SECONDS) - time) / float(SECONDS));
                }
                else if (speed_matric == "WPS")
                {
                    speed = ((charCount / AVERAGE_WORD_LENGTH) - errorCount) / (float(SECONDS) - time);
                }
                else if (speed_matric == "CPM")
                {
                    speed = (charCount - charError) / ((float(SECONDS) - time) / float(SECONDS));
                }
                else if (speed_matric == "CPS")
                {
                    speed = (charCount - charError) / (float(SECONDS) - time);
                }
                else
                {
                    speed = ((charCount / AVERAGE_WORD_LENGTH) - errorCount) / ((float(SECONDS) - time) / float(SECONDS));
                }

                speed = round2(speed);

                float accuracy = float(charCount - charError) * 100 / float(charCount);

                accuracy = round2(accuracy);
                accuracylabel->setText(tgui::String(accuracy) + " %");

                speedlabel->setText(tgui::String(speed) + " " + speed_matric);
            }
        }

        window.clear();
        gui.draw();
        window.display();
    }

    return EXIT_SUCCESS;
}

int myrandom(int i) { return rand() % i; }

float round2(float var)
{
    float value = (int)(var * 100 + .5);
    return (float)value / 100;
}

void timeEnd()
{
    wordsbox->removeAllWidgets();

    timer->setEnabled(false);
}

void timeHide()
{
    if (timelabel->getRenderer()->getTextColor() != tgui::Color::Transparent)
    {
        timelabel->getRenderer()->setTextColor(tgui::Color::Transparent);
    }
    else
    {
        timelabel->getRenderer()->setTextColor(tgui::Color::White);
    }

    typebox->setFocused(true);
}

void resize()
{
    vector<Widget::Ptr> widgets = wordsbox->getWidgets();
    int left = 0;
    int top = 0;
    
    for (int i = 0; i < wordCount; i++)
    {
        widgets[i]->setPosition(widgets[i]->getPosition().x, widgets[i]->getPosition().y - WORD_TOP_MARGIN);
    }
    
    for (int i = wordCount; i < widgets.size(); i++)
    {
        int x = widgets[i]->getSize().x;

        if (left + x + WORD_RIGHT_MARGIN >= wordsbox->getSize().x - wordsbox->getRenderer()->getPadding().getRight())
        {
            top += WORD_TOP_MARGIN;
            left = 0;
        }

        widgets[i]->setPosition(left, top);
        left += x + WORD_RIGHT_MARGIN;
    }
}

void reset()
{
    loadingPic->setVisible(true);
    wordsbox->setVisible(false);
    metricPanel->setVisible(false);
    inputPanel->setVisible(false);

    window.clear();
    gui.draw();
    window.display();

    ifstream fin("words.dat");
    if (fin)
    {
        string word;
        vector<string> words;

        while (!fin.eof())
        {
            fin >> word;
            words.push_back(word);
        }

        random_shuffle(words.begin(), words.end(), myrandom);

        int left = 0;
        int top = 0;

        wordsbox->removeAllWidgets();
        for (int i = 0; i < words.size(); i++)
        {
            Label::Ptr word = Label::create(words[i]);
            word->setTextSize(WORD_TEXT_SIZE);
            word->getRenderer()->setTextColor(tgui::Color::White);
            wordsbox->add(word);

            int x = word->getSize().x;

            if (left + x + WORD_RIGHT_MARGIN >= wordsbox->getSize().x - wordsbox->getRenderer()->getPadding().getRight())
            {
                top += WORD_TOP_MARGIN;
                left = 0;
            }

            word->setPosition(left, top);

            left += x + WORD_RIGHT_MARGIN;
        }
        wordsbox->getWidgets()[0]->cast<Label>()->getRenderer()->setBackgroundColor(tgui::Color("#2b2f31"));
        fin.close();
    }

    loadingPic->setVisible(false);
    wordsbox->setVisible(true);
    inputPanel->setVisible(true);
    metricPanel->setVisible(true);

    timelabel->setText(tgui::String(SECONDS));

    typebox->setText("");
    typebox->setEnabled(true);
    typebox->setFocused(true);

    charCount = 0;
    charError = 0;
    wordCount = 0;
    errorCount = 0;

    timer->setEnabled(false);
}

void speedMatricChange()
{
    string prev_speed_label = speed_matric;

    if (speed_matric == "WPM")
    {
        speed_matric = "WPS";
    }
    else if (speed_matric == "WPS")
    {
        speed_matric = "CPM";
    }
    else if (speed_matric == "CPM")
    {
        speed_matric = "CPS";
    }
    else if (speed_matric == "CPS")
    {
        speed_matric = "WPM";
    }

    if (speedlabel->getText() == "0 " + prev_speed_label)
    {
        speedlabel->setText("0 " + speed_matric);
    }

    typebox->setFocused(true);
}

void process()
{
    if (charCount == 0)
    {
        timer->setEnabled(true);
        timer->restart();
    }

    vector<Widget::Ptr> widgets = wordsbox->getWidgets();

    tgui::String text = typebox->getText();

    Label::Ptr currLabel = NULL;
    LabelRenderer* currLabelRenderer = NULL;
    if (text[text.length() - 1] == ' ')
    {
        if (widgets.size())
        {
            currLabel = widgets[wordCount]->cast<Label>();
            currLabelRenderer = currLabel->getRenderer();

            if (text[0] != ' ')
            {
                Label::Ptr nextLabel = widgets[wordCount + 1]->cast<Label>();
                LabelRenderer* nextLabelRenderer = nextLabel->getRenderer();

                currLabelRenderer->setBackgroundColor(tgui::Color::Transparent);
                nextLabelRenderer->setBackgroundColor(tgui::Color("#2b2f31"));
                if (text == currLabel->getText() + " ")
                {
                    currLabelRenderer->setTextColor(tgui::Color::Green);
                }
                else
                {
                    currLabelRenderer->setTextColor(tgui::Color::Red);
                    errorCount++;

                    if (speed_matric == "WPM" or speed_matric == "WPS")
                    {
                        errorlabel->setText(tgui::String(errorCount));
                    }
                    else if (speed_matric == "CPM" or speed_matric == "CPS")
                    {
                        errorlabel->setText(tgui::String(charError));
                    }
                    else
                    {
                        errorlabel->setText(tgui::String(errorCount));
                    }
                }

                wordCount++;

                if (nextLabel->getPosition().y > 0)
                {
                    for (int i = 0; i < widgets.size(); i++)
                    {
                        widgets[i]->setPosition(widgets[i]->getPosition().x, widgets[i]->getPosition().y - WORD_TOP_MARGIN);
                    }
                }
            }
        }
        typebox->setText("");
    }
    else
    {
        if (widgets.size())
        {
            currLabel = widgets[wordCount]->cast<Label>();
            currLabelRenderer = currLabel->getRenderer();

            charCount++;
            if (text == currLabel->getText().substr(0, text.size()))
            {
                currLabelRenderer->setTextColor(tgui::Color::White);
            }
            else
            {
                currLabelRenderer->setTextColor(tgui::Color::Red);
                charError++;
            }
        }
    }
}