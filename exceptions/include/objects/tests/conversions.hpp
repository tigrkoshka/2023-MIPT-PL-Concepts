TEST(Conversion, Simple) {
    // NOLINTNEXTLINE(misc-const-correctness)
    AutoObject<Plain> sample_plain = "plain";
    // NOLINTNEXTLINE(misc-const-correctness)
    AutoObject<Explicit> sample_explicit{"explicit"};
    const AutoObject<Noexcept> sample_noexcept{"noexcept"};

    // explicit

    [[maybe_unused]] const bool bool_1 = static_cast<bool>(sample_plain);
    [[maybe_unused]] const bool bool_2 = static_cast<bool>(sample_explicit);
    [[maybe_unused]] const bool bool_3 = static_cast<bool>(sample_noexcept);

    // implicit

    [[maybe_unused]] const bool bool_4 = sample_plain;
    // bool sample_explicit_bool_fail = sample_explicit;
    [[maybe_unused]] const bool bool_5 = sample_noexcept;

    // contextual

    if (sample_plain && sample_explicit && sample_noexcept) {
        // OK
    }

    // static checks

    static_assert(std::is_constructible_v<bool, AutoObject<Plain>>);
    static_assert(std::is_constructible_v<bool, AutoObject<Explicit>>);
    static_assert(std::is_constructible_v<bool, AutoObject<Noexcept>>);

    static_assert(std::is_convertible_v<AutoObject<Plain>, bool>);
    static_assert(not std::is_convertible_v<AutoObject<Explicit>, bool>);
    static_assert(std::is_convertible_v<AutoObject<Noexcept>, bool>);

    // noexcept checks

    static_assert(not std::is_nothrow_constructible_v<bool, AutoObject<Plain>>);
    static_assert(
        not std::is_nothrow_constructible_v<bool, AutoObject<Explicit>>);
    static_assert(std::is_nothrow_constructible_v<bool, AutoObject<Noexcept>>);

    static_assert(not std::is_nothrow_convertible_v<AutoObject<Plain>, bool>);
    static_assert(
        not std::is_nothrow_convertible_v<AutoObject<Explicit>, bool>);
    static_assert(std::is_nothrow_convertible_v<AutoObject<Noexcept>, bool>);
}
