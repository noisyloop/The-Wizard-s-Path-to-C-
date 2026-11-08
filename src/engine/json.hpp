// Minimal JSON reader (enough for realm packs). No external dependencies.
#pragma once
#include <cstring>
#include <string>
#include <utility>
#include <vector>

struct Json {
    enum Type { Null, Bool, Num, Str, Arr, Obj } t = Null;
    bool b = false;
    double n = 0;
    std::string s;
    std::vector<Json> a;
    std::vector<std::pair<std::string, Json>> o;

    const Json& operator[](const std::string& k) const {
        static const Json nul;
        for (auto& kv : o) if (kv.first == k) return kv.second;
        return nul;
    }
    const Json& operator[](size_t i) const {
        static const Json nul;
        return i < a.size() ? a[i] : nul;
    }
    bool has(const std::string& k) const { for (auto& kv : o) if (kv.first == k) return true; return false; }
    size_t size() const { return t == Arr ? a.size() : o.size(); }
    std::string str(const std::string& def = "") const { return t == Str ? s : def; }
    std::vector<std::string> strs() const {
        std::vector<std::string> v;
        for (auto& e : a) if (e.t == Str) v.push_back(e.s);
        return v;
    }
};

class JsonParser {
    const std::string& src; size_t i = 0; std::string err;
    void ws() { while (i < src.size() && isspace((unsigned char)src[i])) i++; }
    bool fail(const std::string& m) { if (err.empty()) err = m + " at offset " + std::to_string(i); return false; }
    static void utf8(std::string& out, unsigned cp) {
        if (cp < 0x80) out += (char)cp;
        else if (cp < 0x800) { out += (char)(0xC0 | (cp >> 6)); out += (char)(0x80 | (cp & 0x3F)); }
        else { out += (char)(0xE0 | (cp >> 12)); out += (char)(0x80 | ((cp >> 6) & 0x3F)); out += (char)(0x80 | (cp & 0x3F)); }
    }
    bool str(std::string& out) {
        if (src[i] != '"') return fail("expected string");
        i++;
        while (i < src.size() && src[i] != '"') {
            char c = src[i++];
            if (c != '\\') { out += c; continue; }
            if (i >= src.size()) return fail("bad escape");
            char e = src[i++];
            switch (e) {
                case 'n': out += '\n'; break; case 't': out += '\t'; break;
                case 'r': out += '\r'; break; case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case 'u': {
                    if (i + 4 > src.size()) return fail("bad \\u");
                    utf8(out, (unsigned)std::stoul(src.substr(i, 4), nullptr, 16)); i += 4; break;
                }
                default: out += e;
            }
        }
        if (i >= src.size()) return fail("unterminated string");
        i++;
        return true;
    }
    bool val(Json& j) {
        ws();
        if (i >= src.size()) return fail("unexpected end");
        char c = src[i];
        if (c == '{') {
            j.t = Json::Obj; i++; ws();
            if (src[i] == '}') { i++; return true; }
            while (true) {
                ws(); std::string k; if (!str(k)) return false; ws();
                if (src[i] != ':') return fail("expected ':'");
                i++;
                Json v; if (!val(v)) return false; j.o.emplace_back(k, std::move(v)); ws();
                if (src[i] == ',') { i++; continue; }
                if (src[i] == '}') { i++; return true; }
                return fail("expected ',' or '}'");
            }
        }
        if (c == '[') {
            j.t = Json::Arr; i++; ws();
            if (src[i] == ']') { i++; return true; }
            while (true) {
                Json v; if (!val(v)) return false; j.a.push_back(std::move(v)); ws();
                if (src[i] == ',') { i++; continue; }
                if (src[i] == ']') { i++; return true; }
                return fail("expected ',' or ']'");
            }
        }
        if (c == '"') { j.t = Json::Str; return str(j.s); }
        if (src.compare(i, 4, "true") == 0) { j.t = Json::Bool; j.b = true; i += 4; return true; }
        if (src.compare(i, 5, "false") == 0) { j.t = Json::Bool; i += 5; return true; }
        if (src.compare(i, 4, "null") == 0) { i += 4; return true; }
        size_t st = i;
        while (i < src.size() && (isdigit((unsigned char)src[i]) || strchr("+-.eE", src[i]))) i++;
        if (st == i) return fail("unexpected character");
        j.t = Json::Num; j.n = std::stod(src.substr(st, i - st));
        return true;
    }
public:
    explicit JsonParser(const std::string& s) : src(s) {}
    bool parse(Json& out, std::string& error) {
        bool ok = val(out);
        if (!ok) error = err;
        return ok;
    }
};
