#include <iostream>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <fstream>
#include <vector>

using namespace std;
using namespace sf;
using namespace tgui;

int charCount = 0;
int charError = 0;
int wordCount = 0;
int errorCount = 0;

RenderWindow window(VideoMode(1280, 800), "Typing");
GuiSFML gui{ window };

float round2(float);
void reset();
void timeEnd();
void resize();
void process();
int myrandom(int);

Timer::Ptr timer = Timer::create(&timeEnd, 60000, false);

int main()
{
    srand(time(NULL));
    
    gui.loadWidgetsFromFile("form.txt");
    gui.onViewChange(&resize);

    Panel::Ptr wordsbox = gui.get<Panel>("wordsbox");
    wordsbox->getRenderer()->setBorderColor(tgui::Color("#264d6e"));
    
    Panel::Ptr metricPanel = gui.get<Panel>("metricPanel");
    metricPanel->getRenderer()->setBorderColor(tgui::Color("#264d6e"));

    reset();

    BitmapButton::Ptr refreshBtn = gui.get<BitmapButton>("refreshBtn");
    refreshBtn->onClick(&reset);

    EditBox::Ptr typebox = gui.get<EditBox>("typebox");
    typebox->onTextChange(&process);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            gui.handleEvent(event);

            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (timer->getNextScheduledTime())
        {
            int time = timer->getNextScheduledTime()->asSeconds();

            if (time != 0)
            {
                Label::Ptr timelabel = gui.get<Label>("time");
                timelabel->setText(tgui::String(time));

                int speed = (charCount / 5.0) / ((60.0 - timer->getNextScheduledTime()->asSeconds()) / 60.0);
                speed -= (errorCount / time);

                float accuracy = float(charCount - charError) * 100 / float(charCount);

                accuracy = round2(accuracy);
                Label::Ptr accuracylabel = gui.get<Label>("accuracy");
                accuracylabel->setText(tgui::String(accuracy) + " %");

                Label::Ptr speedlabel = gui.get<Label>("speed");
                speedlabel->setText(tgui::String(speed) + " WPM");
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
    Label::Ptr time = gui.get<Label>("time");
    time->setText(tgui::String(60));

    EditBox::Ptr typebox = gui.get<EditBox>("typebox");
    typebox->setEnabled(false);

    timer->setEnabled(false);
}

void resize()
{
    Panel::Ptr wordsbox = gui.get<Panel>("wordsbox");

    vector<Widget::Ptr> widgets = wordsbox->getWidgets();
    int left = 0;
    int top = 0;
    
    for (int i = 0; i < wordCount; i++)
    {
        widgets[i]->setPosition(widgets[i]->getPosition().x, widgets[i]->getPosition().y - 50);
    }
    
    for (int i = wordCount; i < widgets.size(); i++)
    {
        int x = widgets[i]->getSize().x;

        if (left + x >= wordsbox->getSize().x - wordsbox->getRenderer()->getPadding().getRight())
        {
            top += 50;
            left = 0;
        }

        widgets[i]->setPosition(left, top);
        left += x + 2;
    }
}

void reset()
{
    Panel::Ptr wordsbox = gui.get<Panel>("wordsbox");
    wordsbox->removeAllWidgets();

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
        for (int i = 0; i < words.size(); i++)
        {
            Label::Ptr word = Label::create(words[i]);
            word->setTextSize(30);
            word->getRenderer()->setTextColor(tgui::Color::White);
            wordsbox->add(word);

            int x = word->getSize().x;

            if (left + x >= wordsbox->getSize().x - wordsbox->getRenderer()->getPadding().getRight())
            {
                top += 50;
                left = 0;
            }

            word->setPosition(left, top);

            left += x + 2;
        }
        wordsbox->getWidgets()[0]->cast<Label>()->getRenderer()->setBackgroundColor(tgui::Color::applyOpacity(tgui::Color::White, 0.25));
        fin.close();
    }

    EditBox::Ptr typebox = gui.get<EditBox>("typebox");
    typebox->setText("");
    typebox->setEnabled(true);
    typebox->setFocused(true);

    charCount = 0;
    charError = 0;
    wordCount = 0;
    errorCount = 0;
    timer->setEnabled(false);

    Label::Ptr timelabel = gui.get<Label>("time");
    timelabel->setText(tgui::String(60));
}

void process()
{
    if (charCount == 0)
    {
        timer->setEnabled(true);
        timer->restart();
    }

    Panel::Ptr wordsbox = gui.get<Panel>("wordsbox");
    vector<Widget::Ptr> widgets = wordsbox->getWidgets();

    EditBox::Ptr typebox = gui.get<EditBox>("typebox");
    tgui::String text = typebox->getText();

    Label::Ptr currLabel = widgets[wordCount]->cast<Label>();
    LabelRenderer* currLabelRenderer = currLabel->getRenderer();
    if (text[text.length() - 1] == ' ')
    {
        if (text[0] != ' ')
        {
            Label::Ptr nextLabel = widgets[wordCount + 1]->cast<Label>();
            LabelRenderer* nextLabelRenderer = nextLabel->getRenderer();

            currLabelRenderer->setBackgroundColor(tgui::Color::Transparent);
            nextLabelRenderer->setBackgroundColor(tgui::Color::applyOpacity(tgui::Color::White, 0.25));
            if (text == currLabel->getText() + " ")
            {
                currLabelRenderer->setTextColor(tgui::Color::Green);
            }
            else
            {
                currLabelRenderer->setTextColor(tgui::Color::Red);
                errorCount++;

                Label::Ptr errorlabel = gui.get<Label>("errors");
                errorlabel->setText(tgui::String(errorCount));
            }

            wordCount++;

            if (nextLabel->getPosition().y > 0)
            {
                for (int i = 0; i < widgets.size(); i++)
                {
                    widgets[i]->setPosition(widgets[i]->getPosition().x, widgets[i]->getPosition().y - 50);
                }
            }
        }
        typebox->setText("");
    }
    else
    {
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