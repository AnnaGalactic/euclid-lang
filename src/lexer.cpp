

static const std::unordered_map<Unicode::String, Wide::Lexer::TokenType> reserved_words(
    []() -> std::unordered_map<Unicode::String, Wide::Lexer::TokenType>
    {
        // Maps reserved words to TokenType enumerated values
        std::unordered_map<Unicode::String, Wide::Lexer::TokenType> result;

        // RESERVED WORD
        result[L"dynamic_cast"] = Wide::Lexer::TokenType::DynamicCast;
        result[L"for"] = Wide::Lexer::TokenType::For;
        result[L"while"] = Wide::Lexer::TokenType::While;
        result[L"do"] = Wide::Lexer::TokenType::Do;
        result[L"continue"] = Wide::Lexer::TokenType::Continue;
        result[L"auto"] = Wide::Lexer::TokenType::Auto;
        result[L"break"] = Wide::Lexer::TokenType::Break;
        result[L"type"] = Wide::Lexer::TokenType::Type;
        result[L"switch"] = Wide::Lexer::TokenType::Switch;
        result[L"case"] = Wide::Lexer::TokenType::Case;
        result[L"default"] = Wide::Lexer::TokenType::Default;
        result[L"try"] = Wide::Lexer::TokenType::Try;
        result[L"catch"] = Wide::Lexer::TokenType::Catch;
        result[L"return"] = Wide::Lexer::TokenType::Return;
        result[L"static"] = Wide::Lexer::TokenType::Static;
        result[L"if"] = Wide::Lexer::TokenType::If;
        result[L"else"] = Wide::Lexer::TokenType::Else;
        result[L"decltype"] = Wide::Lexer::TokenType::Decltype;
        result[L"partial"] = Wide::Lexer::TokenType::Partial;
        result[L"using"] = Wide::Lexer::TokenType::Using;
        result[L"true"] = Wide::Lexer::TokenType::True;
        result[L"false"] = Wide::Lexer::TokenType::False;
        result[L"null"] = Wide::Lexer::TokenType::Null;
        result[L"int"] = Wide::Lexer::TokenType::Int;
        result[L"long"] = Wide::Lexer::TokenType::Long;
        result[L"short"] = Wide::Lexer::TokenType::Short;
        result[L"module"] = Wide::Lexer::TokenType::Module;
        result[L"dynamic"] = Wide::Lexer::TokenType::Dynamic;
        result[L"reinterpret_cast"] = Wide::Lexer::TokenType::ReinterpretCast;
        result[L"static_cast"] = Wide::Lexer::TokenType::StaticCast;
        result[L"enum"] = Wide::Lexer::TokenType::Enum;
        result[L"operator"] = Wide::Lexer::TokenType::Operator;
        result[L"throw"] = Wide::Lexer::TokenType::Throw;
        result[L"public"] = Wide::Lexer::TokenType::Public;
        result[L"private"] = Wide::Lexer::TokenType::Private;
        result[L"protected"] = Wide::Lexer::TokenType::Protected;
        result[L"friend"] = Wide::Lexer::TokenType::Friend;
        result[L"this"] = Wide::Lexer::TokenType::This;

        return result;
    }()
);

std::vector<Wide::Lexer::Token*> Lexer::Context::operator()(Unicode::String* filename, Memory::Arena& arena) {

    Wide::IO::TextInputFileOpenArguments args;
    args.encoding = Wide::IO::Encoding::UTF16;
    args.mode = Wide::IO::OpenMode::OpenExisting;
    args.path = *filename;

    auto str = arena.Allocate<Unicode::String>(args().AsString());
    const wchar_t* begin = str->c_str();
    const wchar_t* end = str->c_str() + str->size();

    int line = 1;
    int column = 1;

    std::vector<Token*> tokens;

    // Some variables we'll need for semantic actions
    Wide::Lexer::TokenType type;

    auto multi_line_comment
        =  MakeEquality(L'/')
        >> MakeEquality(L'*')
        >> *( !(MakeEquality(L'*') >> MakeEquality(L'/')) >> eps)
        >> eps >> eps;

    auto single_line_comment
        =  MakeEquality(L'/')
        >> MakeEquality(L'/')
        >> *( !MakeEquality(L'\n') >> eps);

    auto punctuation
        =  MakeEquality(L',')[[&]{ type = Wide::Lexer::TokenType::Comma; }]
        || MakeEquality(L';')[[&]{ type = Wide::Lexer::TokenType::Semicolon; }]
        || MakeEquality(L'~')[[&]{ type = Wide::Lexer::TokenType::BinaryNOT; }]
        || MakeEquality(L'(')[[&]{ type = Wide::Lexer::TokenType::OpenBracket; }]
        || MakeEquality(L')')[[&]{ type = Wide::Lexer::TokenType::CloseBracket; }]
        || MakeEquality(L'[')[[&]{ type = Wide::Lexer::TokenType::OpenSquareBracket; }]
        || MakeEquality(L']')[[&]{ type = Wide::Lexer::TokenType::CloseSquareBracket; }]
        || MakeEquality(L'{')[[&]{ type = Wide::Lexer::TokenType::OpenCurlyBracket; }]
        || MakeEquality(L'}')[[&]{ type = Wide::Lexer::TokenType::CloseCurlyBracket; }]

        || MakeEquality(L'>') >> (
               MakeEquality(L'>') >> (
                   MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::RightShiftEquals; }]
                || opt[[&]{ type = Wide::Lexer::TokenType::RightShift; }])
            || MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::GreaterThanOrEqualTo; }]
            || opt[[&]{ type = Wide::Lexer::TokenType::GreaterThan; }])
        || MakeEquality(L'<') >> (
               MakeEquality(L'<') >> (
                      MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::LeftShiftEquals; }]
                   || opt[[&]{ type = Wide::Lexer::TokenType::LeftShift; }] )
            || MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::LessThanOrEqualTo; }]
            || opt[[&]{ type = Wide::Lexer::TokenType::LessThan; }])

        || MakeEquality(L'-') >> (
               MakeEquality(L'-')[[&]{ type = Wide::Lexer::TokenType::Decrement; }]
            || MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::MinusEquals; }]
            || MakeEquality(L'>')[[&]{ type = Wide::Lexer::TokenType::PointerAccess; }]
            || opt[[&]{ type = Wide::Lexer::TokenType::Minus; }])

        || MakeEquality(L'.')
            >> (MakeEquality(L'.') >> MakeEquality(L'.')[[&]{ type = Wide::Lexer::TokenType::Ellipsis; }]
            || opt[[&]{ type = Wide::Lexer::TokenType::Dot; }])

        || MakeEquality(L'+') >> (
               MakeEquality(L'+')[[&]{ type = Wide::Lexer::TokenType::Increment; }]
            || MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::PlusEquals; }]
            || opt[[&]{ type = Wide::Lexer::TokenType::Plus; }])
        || MakeEquality(L'&') >> (
               MakeEquality(L'&')[[&]{ type = Wide::Lexer::TokenType::LogicalAnd; }]
            || MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::BinaryANDEquals; }]
            || opt[[&]{ type = Wide::Lexer::TokenType::BinaryAND; }])
        || MakeEquality(L'|') >> (
               MakeEquality(L'|')[[&]{ type = Wide::Lexer::TokenType::LogicalOr; }]
            || MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::BinaryOREquals; }]
            || opt[[&]{ type = Wide::Lexer::TokenType::BinaryOR; }])

        || MakeEquality(L'*') >> (MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::MulEquals; }]
            || opt[[&]{ type = Wide::Lexer::TokenType::Multiply; }])
        || MakeEquality(L'%') >> (MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::ModulusEquals; }]
            || opt[[&]{ type = Wide::Lexer::TokenType::Modulus; }])
        || MakeEquality(L'=') >> (MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::EqualTo; }]
            || opt[[&]{ type = Wide::Lexer::TokenType::Assignment; }])
        || MakeEquality(L'!') >> (MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::NotEquals; }]
            || opt[[&]{ type = Wide::Lexer::TokenType::LogicalNOT; }])
        || MakeEquality(L'/') >> (MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::DivEquals; }]
            || opt[[&]{ type = Wide::Lexer::TokenType::Divide; }])
        || MakeEquality(L'^') >> (MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::BinaryXOREquals; }]
            || opt[[&]{ type = Wide::Lexer::TokenType::BinaryXOR; }])
        || MakeEquality(L':') >> (MakeEquality(L'=')[[&]{ type = Wide::Lexer::TokenType::VarAssign; }]
            || opt[[&]{ type = Wide::Lexer::TokenType::Colon; }]);

    auto string
        =  L'"' >> *( L'\\' >> MakeEquality(L'"') >> eps || !MakeEquality(L'"') >> eps) >> eps;

    auto character
        =  L'\'' >> *( L'\\' >> MakeEquality(L'\'') >> eps || !MakeEquality(L'\'') >> eps);

    auto digit
        =  MakeRange(L'0', L'9');

    auto letter
        =  MakeRange(L'a', L'z') || MakeRange(L'A', L'Z');

    auto number
        =  +digit >> ((L'.' >> +digit) || opt);

    auto new_line
        = MakeEquality(L'\n')[ [&] { line++; column = 0; } ];

    auto whitespace
        =  MakeEquality(L' ')
        || L'\t'
        || new_line
        || L'\n'
        || L'\r'
        || multi_line_comment
        || single_line_comment;

    auto identifier
        =  (letter || L'_') >> *(letter || digit || (L'_'));
        //=  *( !(punctuation || string || character || whitespace) >> eps );

    bool skip = false;

    auto lexer
        =  whitespace[ [&]{ skip = true; } ] // Do not produce a token for whitespace or comments. Just continue on.
        || punctuation[ [&]{ skip = false; } ] // Type set by individual punctuation
        || string[ [&]{ skip = false; type = Wide::Lexer::TokenType::String; } ]
        || character[ [&]{ skip = false; type = Wide::Lexer::TokenType::Character; } ]
        || number[ [&]{ skip = false; type = Wide::Lexer::TokenType::Number; } ]
        || identifier[ [&]{ skip = false; type = Wide::Lexer::TokenType::Identifier; } ];

    auto current = begin;
    while(current != end) {
        if (!lexer(current, end)) {
            throw std::runtime_error("Failed to lex input.");
        }
        column += (current - begin);
        if (skip) {
            begin = current;
            continue;
        }
        Token t(begin, current);
        t.columnbegin = column - (current - begin);
        t.columnend = column;
        t.file = filename;
        t.line = line;
        if (type == Wide::Lexer::TokenType::Identifier) { // check for reserved word
            if (reserved_words.find(t.Codepoints()) != reserved_words.end())
                t.type = reserved_words.find(t.Codepoints())->second;
            else
                t.type = Wide::Lexer::TokenType::Identifier;
        } else {
            t.type = type;
        }
        begin = current;
        tokens.push_back(arena.Allocate<Token>(t));
    }
    return tokens;
}
