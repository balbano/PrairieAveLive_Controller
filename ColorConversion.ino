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
