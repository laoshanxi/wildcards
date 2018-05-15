// Copyright Tomas Zeman 2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef WILDCARDS_MATCH_HPP
#define WILDCARDS_MATCH_HPP

#include <stdexcept>    // std::invalid_argument, std::logic_error
#include <type_traits>  // std::enable_if, std::is_same
#include <utility>      // std::forward

#include "config.hpp"             // cfg_HAS_CONSTEXPR14
#include "cx/functional.hpp"      // cx::equal_to
#include "cx/iterator.hpp"        // cx::cbegin, cx::cend, cx::next
#include "wildcards/cards.hpp"    // wildcards::cards
#include "wildcards/utility.hpp"  // wildcards::container_item_t, wildcards::iterated_item_t

namespace wildcards
{

namespace detail
{

#if !cfg_HAS_FULL_FEATURED_CONSTEXPR_SWITCH

constexpr bool throw_logic_error(const char* what_arg)
{
  return what_arg == nullptr ? false : throw std::logic_error(what_arg);
}

template <typename T>
constexpr T throw_logic_error(T t, const char* what_arg)
{
  return what_arg == nullptr ? t : throw std::logic_error(what_arg);
}

constexpr bool throw_invalid_argument(const char* what_arg)
{
  return what_arg == nullptr ? false : throw std::invalid_argument(what_arg);
}

template <typename T>
constexpr T throw_invalid_argument(T t, const char* what_arg)
{
  return what_arg == nullptr ? t : throw std::invalid_argument(what_arg);
}

#endif

enum class is_set_state
{
  open,
  not_or_first,
  first,
  next
};

template <typename PatternIterator>
constexpr bool is_set(
    PatternIterator p, PatternIterator pend,
    const cards<iterated_item_t<PatternIterator>>& c = cards<iterated_item_t<PatternIterator>>(),
    is_set_state state = is_set_state::open)
{
#if cfg_HAS_CONSTEXPR14

  if (!c.set_enabled || p == pend)
  {
    return false;
  }

  switch (state)
  {
    case is_set_state::open:
      if (*p == c.set_open)
      {
        return is_set(cx::next(p), pend, c, is_set_state::not_or_first);
      }

      return false;

    case is_set_state::not_or_first:
      if (*p == c.set_not)
      {
        return is_set(cx::next(p), pend, c, is_set_state::first);
      }

      return is_set(cx::next(p), pend, c, is_set_state::next);

    case is_set_state::first:
      return is_set(cx::next(p), pend, c, is_set_state::next);

    case is_set_state::next:
      if (*p == c.set_close)
      {
        return true;
      }

      return is_set(cx::next(p), pend, c, is_set_state::next);

    default:
#if cfg_HAS_FULL_FEATURED_CONSTEXPR_SWITCH
      throw std::logic_error(
          "The program execution should never end up here throwing this exception");
#else
      return throw_logic_error(
          "The program execution should never end up here throwing this exception");
#endif
  }

#else  // !cfg_HAS_CONSTEXPR14

  return c.set_enabled && p != pend &&
         (state == is_set_state::open
              ? *p == c.set_open && is_set(cx::next(p), pend, c, is_set_state::not_or_first)
              :

              state == is_set_state::not_or_first
                  ? *p == c.set_not ? is_set(cx::next(p), pend, c, is_set_state::first)
                                    : is_set(cx::next(p), pend, c, is_set_state::next)
                  : state == is_set_state::first
                        ? is_set(cx::next(p), pend, c, is_set_state::next)
                        : state == is_set_state::next
                              ? *p == c.set_close ||
                                    is_set(cx::next(p), pend, c, is_set_state::next)
                              : throw std::logic_error("The program execution should never end up "
                                                       "here throwing this exception"));

#endif  // cfg_HAS_CONSTEXPR14
}

enum class skip_set_state
{
  open,
  not_or_first,
  first,
  next
};

template <typename PatternIterator>
constexpr PatternIterator skip_set(
    PatternIterator p, PatternIterator pend,
    const cards<iterated_item_t<PatternIterator>>& c = cards<iterated_item_t<PatternIterator>>(),
    skip_set_state state = skip_set_state::open)
{
#if cfg_HAS_CONSTEXPR14

  if (!c.set_enabled)
  {
#if cfg_HAS_FULL_FEATURED_CONSTEXPR_SWITCH
    throw std::invalid_argument("The use of sets is disabled");
#else
    return throw_invalid_argument(p, "The use of sets is disabled");
#endif
  }

  if (p == pend)
  {
#if cfg_HAS_FULL_FEATURED_CONSTEXPR_SWITCH
    throw std::invalid_argument("The given pattern is not a valid set");
#else
    return throw_invalid_argument(p, "The given pattern is not a valid set");
#endif
  }

  switch (state)
  {
    case skip_set_state::open:
      if (*p == c.set_open)
      {
        return skip_set(cx::next(p), pend, c, skip_set_state::not_or_first);
      }

#if cfg_HAS_FULL_FEATURED_CONSTEXPR_SWITCH
      throw std::invalid_argument("The given pattern is not a valid set");
#else
      return throw_invalid_argument(p, "The given pattern is not a valid set");
#endif

    case skip_set_state::not_or_first:
      if (*p == c.set_not)
      {
        return skip_set(cx::next(p), pend, c, skip_set_state::first);
      }

      return skip_set(cx::next(p), pend, c, skip_set_state::next);

    case skip_set_state::first:
      return skip_set(cx::next(p), pend, c, skip_set_state::next);

    case skip_set_state::next:
      if (*p == c.set_close)
      {
        return cx::next(p);
      }

      return skip_set(cx::next(p), pend, c, skip_set_state::next);

    default:
#if cfg_HAS_FULL_FEATURED_CONSTEXPR_SWITCH
      throw std::logic_error(
          "The program execution should never end up here throwing this exception");
#else
      return throw_logic_error(
          p, "The program execution should never end up here throwing this exception");
#endif
  }

#else  // !cfg_HAS_CONSTEXPR14

  return !c.set_enabled
             ? throw std::invalid_argument("The use of sets is disabled")
             : p == pend
                   ? throw std::invalid_argument("The given pattern is not a valid set")
                   :

                   state == skip_set_state::open
                       ? *p == c.set_open
                             ? skip_set(cx::next(p), pend, c, skip_set_state::not_or_first)
                             : throw std::invalid_argument("The given pattern is not a valid set")
                       :

                       state == skip_set_state::not_or_first
                           ? *p == c.set_not ? skip_set(cx::next(p), pend, c, skip_set_state::first)
                                             : skip_set(cx::next(p), pend, c, skip_set_state::next)
                           : state == skip_set_state::first
                                 ? skip_set(cx::next(p), pend, c, skip_set_state::next)
                                 : state == skip_set_state::next
                                       ? *p == c.set_close
                                             ? cx::next(p)
                                             : skip_set(cx::next(p), pend, c, skip_set_state::next)
                                       : throw std::logic_error(
                                             "The program execution should never end up "
                                             "here throwing this exception");

#endif  // cfg_HAS_CONSTEXPR14
}

enum class match_set_state
{
  open,
  not_or_first_in,
  first_out,
  skip_next_in,
  next_in,
  next_out
};

template <typename SequenceIterator, typename PatternIterator,
          typename EqualTo = cx::equal_to<void>>
constexpr bool match_set(
    SequenceIterator s, SequenceIterator send, PatternIterator p, PatternIterator pend,
    const cards<iterated_item_t<PatternIterator>>& c = cards<iterated_item_t<PatternIterator>>(),
    const EqualTo& equal_to = EqualTo(), match_set_state state = match_set_state::open)
{
#if cfg_HAS_CONSTEXPR14

  if (!c.set_enabled)
  {
#if cfg_HAS_FULL_FEATURED_CONSTEXPR_SWITCH
    throw std::invalid_argument("The use of sets is disabled");
#else
    return throw_invalid_argument("The use of sets is disabled");
#endif
  }

  if (p == pend)
  {
#if cfg_HAS_FULL_FEATURED_CONSTEXPR_SWITCH
    throw std::invalid_argument("The given pattern is not a valid set");
#else
    return throw_invalid_argument("The given pattern is not a valid set");
#endif
  }

  switch (state)
  {
    case match_set_state::open:
      if (*p == c.set_open)
      {
        return match_set(s, send, cx::next(p), pend, c, equal_to, match_set_state::not_or_first_in);
      }

#if cfg_HAS_FULL_FEATURED_CONSTEXPR_SWITCH
      throw std::invalid_argument("The given pattern is not a valid set");
#else
      return throw_invalid_argument("The given pattern is not a valid set");
#endif

    case match_set_state::not_or_first_in:
      if (*p == c.set_not)
      {
        return match_set(s, send, cx::next(p), pend, c, equal_to, match_set_state::first_out);
      }

      if (s == send)
      {
        return false;
      }

      if (equal_to(*s, *p))
      {
        return match_set(s, send, cx::next(p), pend, c, equal_to, match_set_state::skip_next_in);
      }

      return match_set(s, send, cx::next(p), pend, c, equal_to, match_set_state::next_in);

    case match_set_state::first_out:
      if (s == send || equal_to(*s, *p))
      {
        return false;
      }

      return match_set(s, send, cx::next(p), pend, c, equal_to, match_set_state::next_out);

    case match_set_state::skip_next_in:
      if (*p == c.set_close)
      {
        if (s == send)
        {
          return true;
        }

        return match(cx::next(s), send, cx::next(p), pend, c, equal_to);
      }

      return match_set(s, send, cx::next(p), pend, c, equal_to, state);

    case match_set_state::next_in:
      if (*p == c.set_close || s == send)
      {
        return false;
      }

      if (equal_to(*s, *p))
      {
        return match_set(s, send, cx::next(p), pend, c, equal_to, match_set_state::skip_next_in);
      }

      return match_set(s, send, cx::next(p), pend, c, equal_to, state);

    case match_set_state::next_out:
      if (*p == c.set_close)
      {
        if (s == send)
        {
          return true;
        }

        return match(cx::next(s), send, cx::next(p), pend, c, equal_to);
      }

      if (s == send || equal_to(*s, *p))
      {
        return false;
      }

      return match_set(s, send, cx::next(p), pend, c, equal_to, state);

    default:
#if cfg_HAS_FULL_FEATURED_CONSTEXPR_SWITCH
      throw std::logic_error(
          "The program execution should never end up here throwing this exception");
#else
      return throw_logic_error(
          "The program execution should never end up here throwing this exception");
#endif
  }

#else  // !cfg_HAS_CONSTEXPR14

  return !c.set_enabled
             ? throw std::invalid_argument("The use of sets is disabled")
             : p == pend
                   ? throw std::invalid_argument("The given pattern is not a valid set")
                   : state == match_set_state::open
                         ? *p == c.set_open
                               ? match_set(s, send, cx::next(p), pend, c, equal_to,
                                           match_set_state::not_or_first_in)
                               :

                               throw std::invalid_argument("The given pattern is not a valid set")
                         :

                         state == match_set_state::not_or_first_in
                             ? *p == c.set_not
                                   ? match_set(s, send, cx::next(p), pend, c, equal_to,
                                               match_set_state::first_out)
                                   :

                                   s != send &&
                                       (equal_to(*s, *p)
                                            ? match_set(s, send, cx::next(p), pend, c, equal_to,
                                                        match_set_state::skip_next_in)
                                            :

                                            match_set(s, send, cx::next(p), pend, c, equal_to,
                                                      match_set_state::next_in))

                             :

                             state == match_set_state::first_out
                                 ? s != send && !equal_to(*s, *p) &&
                                       match_set(s, send, cx::next(p), pend, c, equal_to,
                                                 match_set_state::next_out)

                                 :

                                 state == match_set_state::skip_next_in
                                     ? *p == c.set_close
                                           ?

                                           s == send || match(cx::next(s), send, cx::next(p), pend,
                                                              c, equal_to)
                                           :

                                           match_set(s, send, cx::next(p), pend, c, equal_to, state)

                                     :

                                     state == match_set_state::next_in
                                         ? *p != c.set_close && s != send &&
                                               (equal_to(*s, *p)
                                                    ?

                                                    match_set(s, send, cx::next(p), pend, c,
                                                              equal_to,
                                                              match_set_state::skip_next_in)
                                                    :

                                                    match_set(s, send, cx::next(p), pend, c,
                                                              equal_to, state))

                                         :

                                         state == match_set_state::next_out
                                             ? *p == c.set_close
                                                   ?

                                                   s == send ||
                                                       match(cx::next(s), send, cx::next(p), pend,
                                                             c, equal_to)
                                                   :

                                                   s != send && !equal_to(*s, *p) &&
                                                       match_set(s, send, cx::next(p), pend, c,
                                                                 equal_to, state)

                                             : throw std::logic_error(
                                                   "The program execution should never end up here "
                                                   "throwing this exception");

#endif  // cfg_HAS_CONSTEXPR14
}

}  // namespace detail

template <typename SequenceIterator, typename PatternIterator,
          typename EqualTo = cx::equal_to<void>>
constexpr bool match(
    SequenceIterator s, SequenceIterator send, PatternIterator p, PatternIterator pend,
    const cards<iterated_item_t<PatternIterator>>& c = cards<iterated_item_t<PatternIterator>>(),
    const EqualTo& equal_to = EqualTo(), bool escape = false)
{
#if cfg_HAS_CONSTEXPR14

  if (p == pend)
  {
    return s == send;
  }

  if (escape)
  {
    if (s != send && equal_to(*s, *p))
    {
      return match(cx::next(s), send, cx::next(p), pend, c, equal_to);
    }

    return false;
  }

  if (*p == c.anything)
  {
    return match(s, send, cx::next(p), pend, c, equal_to) ||
           (s != send && match(cx::next(s), send, p, pend, c, equal_to));
  }

  if (*p == c.single)
  {
    return s != send && match(cx::next(s), send, cx::next(p), pend, c, equal_to);
  }

  if (*p == c.escape)
  {
    return match(s, send, cx::next(p), pend, c, equal_to, true);
  }

  if (c.set_enabled && *p == c.set_open &&
      detail::is_set(cx::next(p), pend, c, detail::is_set_state::not_or_first))
  {
    return match_set(s, send, cx::next(p), pend, c, equal_to,
                     detail::match_set_state::not_or_first_in);
  }

  if (s != send && equal_to(*s, *p))
  {
    return match(cx::next(s), send, cx::next(p), pend, c, equal_to);
  }

  return false;

#else  // !cfg_HAS_CONSTEXPR14

  return p == pend
             ? s == send
             : escape
                   ? s != send && equal_to(*s, *p) &&
                         match(cx::next(s), send, cx::next(p), pend, c, equal_to)
                   : *p == c.anything
                         ? match(s, send, cx::next(p), pend, c, equal_to) ||
                               (s != send && match(cx::next(s), send, p, pend, c, equal_to))
                         : *p == c.single
                               ? s != send &&
                                     match(cx::next(s), send, cx::next(p), pend, c, equal_to)
                               : *p == c.escape
                                     ? match(s, send, cx::next(p), pend, c, equal_to, true)
                                     : c.set_enabled && *p == c.set_open &&
                                               detail::is_set(cx::next(p), pend, c,
                                                              detail::is_set_state::not_or_first)
                                           ? match_set(s, send, cx::next(p), pend, c, equal_to,
                                                       detail::match_set_state::not_or_first_in)
                                           : s != send && equal_to(*s, *p) &&
                                                 match(cx::next(s), send, cx::next(p), pend, c,
                                                       equal_to);

#endif  // cfg_HAS_CONSTEXPR14
}

template <typename Sequence, typename Pattern, typename EqualTo = cx::equal_to<void>>
constexpr bool match(Sequence&& sequence, Pattern&& pattern,
                     const cards<container_item_t<Pattern>>& c = cards<container_item_t<Pattern>>(),
                     const EqualTo& equal_to = EqualTo())
{
  return match(cx::cbegin(sequence), cx::cend(std::forward<Sequence>(sequence)),
               cx::cbegin(pattern), cx::cend(std::forward<Pattern>(pattern)), c, equal_to);
}

template <typename Sequence, typename Pattern, typename EqualTo = cx::equal_to<void>,
          typename = typename std::enable_if<!std::is_same<EqualTo, cards_type>::value>::type>
constexpr bool match(Sequence&& sequence, Pattern&& pattern, const EqualTo& equal_to)
{
  return match(std::forward<Sequence>(sequence), std::forward<Pattern>(pattern),
               cards<container_item_t<Pattern>>(), equal_to);
}

}  // namespace wildcards

#endif  // WILDCARDS_MATCH_HPP
