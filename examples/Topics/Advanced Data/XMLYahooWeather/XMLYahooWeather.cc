/**
 * Loading XML Data
 * by Daniel Shiffman.
 *
 * This example demonstrates how to use loadXML()
 * to retrieve data from an XML document via a URL
 */

// We're going to store the temperature
int temperature = 0;
// We're going to store text about the weather
std::string weather = "";

// Direct latitude/longitude coordinates (for East Village, NYC - similar to ZIP 10003)
std::string latitude = "40.731";
std::string longitude = "-73.989";

// Location name for display
std::string locationName = "East Village, NYC";


PFont font;

void setup() {
  size(600, 360);

  font = createFont("Merriweather-Light.ttf", 28);
  textFont(font);

  // The URL for the XML document
  std::string url = "https://forecast.weather.gov/MapClick.php?lat=" + latitude + "&lon=" + longitude + "&FcstType=dwml";

  // Load the XML document
  XML xml = loadXML(url);

  // Navigate through the XML structure to get temperature and weather
  XML parameters = xml.getChild("data").getChild("parameters");

  // Get temperature from the first temperature element (usually the daily maximum)
  XML tempElement = parameters.getChild("temperature");
  std::vector<XML> values = tempElement.getChildren("value");
  if (values.size() > 0) {
    temperature = values[0].getIntContent();
  }

  // Get weather description
  XML weatherElement = parameters.getChild("weather");
  XML weatherCondition = weatherElement.getChild("weather-conditions");
  if (weatherCondition.element != nullptr) {
    weather = weatherCondition.getString("weather-summary");
  }

}

void draw() {
  background(255);
  fill(0);

  // Display all the stuff we want to display
  text(fmt::format("Place: {}", locationName), width*0.15, height*0.33);
  text(fmt::format("Today's high: {}F", temperature), width*0.15, height*0.5);
  text(fmt::format("Forecast: {}", weather), width*0.15, height*0.66);

}
