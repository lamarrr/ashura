
  bool is_flexible() const noexcept {
    bool flexible_dependent_width = f32_eq(
        std::holds_alternative<DependentParameters>(get_self_layout().width) &&
            std::get<DependentParameters>(get_self_layout().width)
                .children_allocation.scale,
        0.0f);

    // is it technically unbounded if it's width depends on the children's?

    // min max same
    bool flexible_independent_width = f32_eq(
        std::holds_alternative<IndependentParameters>(
            get_self_layout().width) &&
            std::get<IndependentParameters>(get_self_layout().width).scale,
        0.0f);

    bool flexible_dependent_height = f32_eq(
        std::holds_alternative<DependentParameters>(get_self_layout().height) &&
            std::get<DependentParameters>(get_self_layout().height)
                .children_allocation.scale,
        0.0f);

    bool flexible_independent_height = f32_eq(
        std::holds_alternative<IndependentParameters>(
            get_self_layout().height) &&
            std::get<IndependentParameters>(get_self_layout().height).scale,
        0.0f);

    return flexible_dependent_height || flexible_dependent_width ||
           flexible_independent_height || flexible_independent_width;
  }