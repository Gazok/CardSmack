#include "Card.hpp"

#include <cmath>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <Tank/System/Game.hpp>
#include "Resources.hpp"
#include "Deck.hpp"

sf::Font CardSlot::cardFont;
bool CardSlot::fontLoaded = false;
const tank::Vectoru CardSlot::dimensions = { 46, 64 };

const double Card::rarityToChance[4] {
    std::exp(3), std::exp(2), std::exp(1), std::exp(0)
};

Card::Card(std::string xmlPath, std::string imagePath, Deck const* deck)
    : deck(deck)
{
    tank::Game::log << "Attempting to load card " << xmlPath
                    << " with image " << imagePath << std::endl;

    boost::property_tree::ptree cardTree;
    boost::property_tree::read_xml(xmlPath, cardTree);

    name_ = cardTree.get("name","unnamed");
    strength_ = cardTree.get<unsigned int>("strength");
    cost_ = cardTree.get<unsigned int>("cost");
    rarity_ = cardTree.get<unsigned int>("rarity");
    relativeChance_ = rarityToChance[rarity_];
    ability_ = static_cast<Ability>(cardTree.get("ability",0));

    imagePath = cardTree.get("image",imagePath);
    image_.load(imagePath);
}

CardSlot::CardSlot(tank::Vectorf pos, Card const* card)
    : ZoomHack(pos)
    , card_(card)
{
    if (not fontLoaded)
    {
        cardFont.loadFromFile("res/fnt/gothic_pixel.ttf");
        fontLoaded = true;
    }

    setCard(card);
}

void CardSlot::setCard(Card const* card)
{
    card_ = card;

    auto& graphics = getGraphicList();
    while (not graphics.empty())
    {
        graphics.pop_back();
    }

    if (card == nullptr)
    {
        return;
    }

    makeGraphic<tank::Image>(card->deck->backing);
    makeGraphic<tank::Image>(card->deck->rarity[card->getRarity()]);

    for (unsigned int i = 0; i < card->getStrength(); ++i)
    {
        makeGraphic<tank::Image>(res::powerIcon)->setOrigin({- 36.f, - 16.f - i * 9.f});
    }

    for (unsigned int i = 0; i < card->getCost(); ++i)
    {
        makeGraphic<tank::Image>(res::costIcon)->setOrigin({- 3.f, - 16.f - i * 9.f});
    }



    if (card->getAbility())
    {
        makeGraphic<tank::Image>(res::abilities[card->getAbility()]);
    }

    text_.setString(card->getName());
    text_.setFont(cardFont);
    text_.setCharacterSize(16);
    makeGraphic<tank::Image>(card->getImage());
}

void CardSlot::draw(tank::Camera const& cam)
{
    ZoomHack::draw(cam);

    if (card_ != nullptr)
    {
        tank::Vectorf pos = (getPos() + tank::Vectorf{dimensions.x / 2.f, 5}) * ZoomHack::getScale();

        sf::FloatRect textRect = text_.getLocalBounds();
        text_.setOrigin(std::floor(textRect.left + textRect.width / 2.f), 0.f);

        float angle = getRotation();

        text_.setPosition({pos.x, pos.y});
        text_.setRotation(angle);

        tank::Game::window()->SFMLWindow().draw(text_);
    }
}
