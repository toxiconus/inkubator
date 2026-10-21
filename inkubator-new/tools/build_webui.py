from pathlib import Path
import re
import shutil

ROOT = Path(__file__).resolve().parent.parent
DATA = ROOT / 'firmware' / 'data'
DEV = DATA / 'dev'

INDEX = DATA / 'index.html'
KAL = DATA / 'kalibracja.html'
TEMPLATE = DEV / 'index.template.html'
STYLES = DEV / 'styles.css'
SCRIPT = DEV / 'app.js'
KAL_DEV = DEV / 'kalibracja.html'

STYLE_MARKER = '<link rel="stylesheet" href="styles.css">'
SCRIPT_MARKER = '<script src="app.js"></script>'


def extract_dev_sources():
    DEV.mkdir(parents=True, exist_ok=True)
    text = INDEX.read_text(encoding='utf-8')

    style_match = re.search(r'<style>(.*?)</style>', text, flags=re.S)
    if not style_match:
        raise SystemExit('No <style>...</style> block found in index.html')
    style_content = style_match.group(1).strip()

    script_match = list(re.finditer(r'<script>(.*?)</script>', text, flags=re.S))
    if not script_match:
        raise SystemExit('No <script>...</script> block found in index.html')
    script_content = script_match[-1].group(1).strip()
    script_block = script_match[-1].group(0)

    template_text = text[:style_match.start()] + STYLE_MARKER + text[style_match.end():script_match[-1].start()] + SCRIPT_MARKER + text[script_match[-1].end():]
    template_text = template_text.replace('\r\n', '\n')

    TEMPLATE.write_text(template_text, encoding='utf-8')
    STYLES.write_text(style_content + '\n', encoding='utf-8')
    SCRIPT.write_text(script_content + '\n', encoding='utf-8')
    if KAL.exists():
        shutil.copy2(KAL, KAL_DEV)
    print(f'Extracted dev sources to {DEV}')
    print(f' - {TEMPLATE.name}')
    print(f' - {STYLES.name}')
    print(f' - {SCRIPT.name}')
    if KAL.exists():
        print(f' - {KAL_DEV.name}')


def build_deployed_index(minify=False):
    if not TEMPLATE.exists() or not STYLES.exists() or not SCRIPT.exists():
        raise SystemExit('Dev sources missing. Run with --extract first or create dev/index.template.html, dev/styles.css and dev/app.js')

    template_text = TEMPLATE.read_text(encoding='utf-8')
    style_content = STYLES.read_text(encoding='utf-8')
    script_content = SCRIPT.read_text(encoding='utf-8')

    if minify:
        style_content = minify_css(style_content)
        script_content = minify_js(script_content)

    deployed = template_text.replace(STYLE_MARKER, f'<style>\n{style_content}\n</style>')
    deployed = deployed.replace(SCRIPT_MARKER, f'<script>\n{script_content}\n</script>')
    INDEX.write_text(deployed, encoding='utf-8')
    print(f'Built deployed {INDEX}')
    if KAL_DEV.exists():
        shutil.copy2(KAL_DEV, KAL)
        print(f'Copied {KAL_DEV} -> {KAL}')


def minify_css(css: str) -> str:
    css = re.sub(r'/\*.*?\*/', '', css, flags=re.S)
    css = re.sub(r'\s+', ' ', css)
    css = re.sub(r'\s*([{}:;,])\s*', r'\1', css)
    return css.strip()


def minify_js(js: str) -> str:
    js = re.sub(r'/\*.*?\*/', '', js, flags=re.S)
    js = re.sub(r'//[^\n]*', '', js)
    js = re.sub(r'\s+', ' ', js)
    return js.strip()


def main():
    import argparse
    parser = argparse.ArgumentParser(description='Build or extract WebUI assets for firmware/data')
    parser.add_argument('--extract', action='store_true', help='Extract dev sources from current firmware/data/index.html')
    parser.add_argument('--build', action='store_true', help='Build firmware/data/index.html from dev sources')
    parser.add_argument('--minify', action='store_true', help='Minify CSS and JS during build')
    args = parser.parse_args()

    if not args.extract and not args.build:
        parser.print_help()
        raise SystemExit(1)

    if args.extract:
        extract_dev_sources()
    if args.build:
        build_deployed_index(minify=args.minify)


if __name__ == '__main__':
    main()
