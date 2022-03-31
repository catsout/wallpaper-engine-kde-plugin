/**
 * @param {Object} codes
*/
function BBCode(codes) {
    this.codes = [];

    this.setCodes(codes);
    this.reFlag = 'igm';
}

/**
 * parse
 *
 * @param {String} text
 * @returns {String}
 */
BBCode.prototype.parse = function(text) {
    return this.codes.reduce((text_out, code) => text_out.replace(code.regexp, code.replacement), text);
}

/**
 * add bb codes
 *
 * @param {String} regex
 * @param {String} replacement
 * @returns {BBCode}
 */
BBCode.prototype.add = function(regex, replacement) {
    this.codes.push({
        regexp:      new RegExp(regex, this.reFlag),
        replacement: replacement
    });

    return this;
}

/**
 * set bb codes
 *
 * @param {Object} codes
 * @returns {BBCode}
 */
BBCode.prototype.setCodes = function(codes) {
    this.codes = Object.keys(codes).map(function (regex) {
        const replacement = codes[regex];

        return {
            regexp:      new RegExp(regex, this.reFlag),
            replacement: replacement
        };
    }, this);

    return this;
}

const _parser = new BBCode({});
_parser.add('\n', '<br>');
_parser.add('\\[br\\]', '<br>');
_parser.add('\\[b\\](.+?)\\[/b\\]', '<strong>$1</strong>');
_parser.add('\\[i\\](.+?)\\[/i\\]', '<em>$1</em>');
_parser.add('\\[u\\](.+?)\\[/u\\]', '<u>$1</u>');

_parser.add('\\[h1\\](.+?)\\[/h1\\]', '<h1>$1</h1>');
_parser.add('\\[h2\\](.+?)\\[/h2\\]', '<h2>$1</h2>');
_parser.add('\\[h3\\](.+?)\\[/h3\\]', '<h3>$1</h3>');
_parser.add('\\[h4\\](.+?)\\[/h4\\]', '<h4>$1</h4>');
_parser.add('\\[h5\\](.+?)\\[/h5\\]', '<h5>$1</h5>');
_parser.add('\\[h6\\](.+?)\\[/h6\\]', '<h6>$1</h6>');

_parser.add('\\[p\\](.+?)\\[/p\\]', '<p>$1</p>');
_parser.add('\\[color=(.+?)\\](.+?)\\[/color\\]',  '<span style="color,$1">$2</span>');
_parser.add('\\[size=([0-9]+)\\](.+?)\\[/size\\]', '<span style="font-size,$1px">$2</span>');

_parser.add('\\[img\\](.+?)\\[/img\\]', '<img src="$1">');
_parser.add('\\[img=(.+?)\\]',          '<img src="$1">');

_parser.add('\\[email\\](.+?)\\[/email\\]',       '<a href="mailto,$1">$1</a>');
_parser.add('\\[email=(.+?)\\](.+?)\\[/email\\]', '<a href="mailto,$1">$2</a>');

_parser.add('\\[url\\](.+?)\\[/url\\]',                    '<a href="$1">$1</a>');
_parser.add('\\[url=(.+?)starget=(.+?)\\](.+?)\\[/url\\]', '<a href="$1" target="$2">$3</a>');
_parser.add('\\[url=(.+?)\\](.+?)\\[/url\\]',              '<a href="$1">$2</a>');

_parser.add('\\[a=(.+?)\\](.+?)\\[/a\\]', '<a href="$1" name="$1">$2</a>');
_parser.add('\\[list\\](.+?)\\[/list\\]', '<ul>$1</ul>');
_parser.add('\\[*\\](.+?)\\[/*\\]',   '<li>$1</li>');

export const parser = _parser;
