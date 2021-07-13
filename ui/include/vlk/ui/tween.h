


double Tween::CalculateValue(Tween::Type type, double state) {
  DCHECK_GE(state, 0);
  DCHECK_LE(state, 1);

  switch (type) {
    case EASE_IN:
      return pow(state, 2);

    case EASE_IN_2:
      return pow(state, 4);

    case EASE_IN_OUT:
      if (state < 0.5)
        return pow(state * 2, 2) / 2.0;
      return 1.0 - (pow((state - 1.0) * 2, 2) / 2.0);

    case FAST_IN_OUT:
      return (pow(state - 0.5, 3) + 0.125) / 0.25;

    case LINEAR:
      return state;

    case EASE_OUT_SNAP:
      state = 0.95 * (1.0 - pow(1.0 - state, 2));
      return state;

    case EASE_OUT:
      return 1.0 - pow(1.0 - state, 2);

    case EASE_OUT_2:
      return 1.0 - pow(1.0 - state, 3);

    case EASE_OUT_3:
      return 1.0 - pow(1.0 - state, 4);

    case SMOOTH_IN_OUT:
      return sin(state);

    case ZERO:
      return 0;
  }

  NOTREACHED();
  return state;
}