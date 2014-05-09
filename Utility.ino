int RGBToHex(int red, int green, int blue)
{
  return (red << 16) + (green << 8) + blue;
}

int hexToRGB(int hexColor, char colorToReturn)
{
  switch (colorToReturn) {
    case 'r':
      return hexColor >> 16;
    case 'g':
      return (hexColor >> 8) & 0xFF;
    case 'b':
      return hexColor & 0xFF;
  }
}

float distance(float x1, float y1, float x2, float y2)
{
  return sqrt(pow(x2-x1, 2) + pow(y2-y1, 2));
}
