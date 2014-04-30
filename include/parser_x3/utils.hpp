#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#define ANNOTATE(Type)\
template <typename iterator_type, typename Attr, typename Context>\
inline void on_success(Type, const iterator_type& first, const iterator_type&, Attr& attr, Context const&){\
    auto& pos = first.get_position();\
    attr.pos.file = pos.file;\
    attr.pos.line = pos.line;\
    attr.pos.column = pos.column;\
}

#endif
