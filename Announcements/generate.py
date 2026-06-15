from pathlib import Path
import markdown
from jinja2 import Environment, select_autoescape
import yaml
from random import randint
from weasyprint import HTML
import fitz
from io import BytesIO
from PIL import Image


templates_folder = Path("templates")
output_folder = Path("output")
output_debug_folder = Path("output") / "debug"
env = Environment(autoescape=select_autoescape(['html', 'xml']))

if not output_folder.exists():
    output_folder.mkdir()

if not output_debug_folder.exists():
    output_debug_folder.mkdir()


def build_random_telephone_number() -> str:
    return ' '.join(
        [
            ''.join([str(randint(0, 9)) for _ in range(3)]),
            ''.join([str(randint(0, 9)) for _ in range(6)])
        ]
    )


def get_random_bw_color() -> str:
    random_int_100 = randint(0, 100)
    return 'rgb(%d%%, %d%%, %d%%)' % (random_int_100, random_int_100, random_int_100)


def build_tearoff_items(n, random_bw_color=False, blank=False):
    items = []

    telephone_number = build_random_telephone_number()

    for i in range(n):
        item = {}

        if not blank:
            item['l1'] = telephone_number

        if random_bw_color:
            item['style'] = 'background-color: %s;' % get_random_bw_color()

        items.append(item)

    return items


def render(html: str, template_name: str):
    pdf_bytes = HTML(string=html).write_pdf()

    pdf_doc = fitz.open(stream=pdf_bytes, filetype="pdf")
    page = pdf_doc[0]

    # saved pdf (debugging)
    pdf_doc.save(str(output_debug_folder / f'{template_name}.pdf'))

    pix = page.get_pixmap(dpi=80, alpha=False, colorspace=fitz.csGRAY)

    image = Image.open(BytesIO(pix.tobytes('png'))).convert('L')

    pdf_doc.close()
    return image


for template_folder in templates_folder.iterdir():

    template_name = template_folder.name
    template_file = template_folder / "template.html"
    template_text = template_file.read_text(encoding="utf-8")
    content_variations_folder = template_folder / "content-variations"
    variables = yaml.safe_load((template_folder / 'vars.yaml').read_text(encoding="utf-8"))

    for content_variation in content_variations_folder.iterdir():

        announcement_text = content_variation.read_text(encoding="utf-8")
        announcement_html = markdown.markdown(announcement_text)

        template = env.from_string(template_text)

        # render the normal announcement html
        normal_render = template.render(announcement_text=announcement_html,
                                        tearoff_items=build_tearoff_items(variables['n_tearoff_items']))

        # save the html for debuggin purposes
        (output_debug_folder / f'{template_name}.html').write_text(normal_render, encoding="utf-8")

        channel_r = render(normal_render, template_name)

        # render the black-and-white id map
        id_map_render = template.render(announcement_text=None,
                                        tearoff_items=build_tearoff_items(variables['n_tearoff_items'], True, True))

        # save the id map html for debugging purposes
        (output_debug_folder / f'{template_name}_id_map.html').write_text(id_map_render, encoding="utf-8")

        channel_g = render(id_map_render, template_name)

        # white texture for blue and alpha
        w, h = channel_r.size
        channel_blue = Image.new('L', (w, h), 255)
        channel_alpha = Image.new('L', (w, h), 255)

        texture = Image.merge('RGBA', (channel_r, channel_g, channel_blue, channel_alpha))

        # the resulting image contains some white pixels at the bottom where they are not wanted - the easiest solution is just cropping
        crop_pxl_number = 1
        texture = texture.crop((crop_pxl_number, 0, w-crop_pxl_number, h-crop_pxl_number))

        texture.save(fp=output_folder / ('%s_%s.png' % (template_name, content_variation.name.replace('.md', ''))), format='png')