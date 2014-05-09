float factorByDistance(float x1, float y1, float x2, float y2, float level, float scalingFactor) {
  // Used for the "loudness gradient" circles around each audio node
  // x1, y1 is the source of the sound. x2, y2, is the position at which to return the factored level.
  level = level * scalingFactor;
  float pointDistance = distance(x1, y1, x2, y2);
  float levelByDistance = map(pointDistance, 0, level, level, 0);
  levelByDistance = constrain(levelByDistance, 0, level);
  return levelByDistance;
}
