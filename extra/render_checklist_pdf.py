from pathlib import Path
import re

from reportlab.lib import colors
from reportlab.lib.enums import TA_LEFT
from reportlab.lib.pagesizes import A4, landscape
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import mm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import Paragraph, SimpleDocTemplate, Spacer, Table, TableStyle


ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "doc" / "documentacao-mbgui" / "08-CHECKLIST-CONFORMIDADE-FUNCIONAL-B8-VS-LAUDO-INATEL-B7.md"
OUT = ROOT / "doc" / "documentacao-mbgui" / "08-CHECKLIST-CONFORMIDADE-FUNCIONAL-B8-VS-LAUDO-INATEL-B7.pdf"


def register_fonts():
    fonts_dir = Path("C:/Windows/Fonts")
    regular = fonts_dir / "arial.ttf"
    bold = fonts_dir / "arialbd.ttf"
    pdfmetrics.registerFont(TTFont("ArialCustom", str(regular)))
    pdfmetrics.registerFont(TTFont("ArialCustom-Bold", str(bold)))


def clean_inline(text: str) -> str:
    text = re.sub(r"\[([^\]]+)\]\([^)]+\)", r"\1", text)
    text = text.replace("`", "")
    return text.strip()


def p(text: str, style):
    return Paragraph(clean_inline(text).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;"), style)


def parse_table(lines, start_idx):
    header = [c.strip() for c in lines[start_idx].strip().strip("|").split("|")]
    rows = []
    idx = start_idx + 2
    while idx < len(lines):
        line = lines[idx]
        if not line.startswith("|"):
            break
        rows.append([c.strip() for c in line.strip().strip("|").split("|")])
        idx += 1
    return [header] + rows, idx


def build_pdf():
    register_fonts()
    styles = getSampleStyleSheet()
    title = ParagraphStyle(
        "TitleCustom",
        parent=styles["Title"],
        fontName="ArialCustom-Bold",
        fontSize=16,
        leading=20,
        alignment=TA_LEFT,
        spaceAfter=8,
    )
    h2 = ParagraphStyle(
        "H2Custom",
        parent=styles["Heading2"],
        fontName="ArialCustom-Bold",
        fontSize=12,
        leading=15,
        spaceBefore=10,
        spaceAfter=6,
    )
    body = ParagraphStyle(
        "BodyCustom",
        parent=styles["BodyText"],
        fontName="ArialCustom",
        fontSize=8.5,
        leading=11,
        spaceAfter=4,
    )
    bullet = ParagraphStyle(
        "BulletCustom",
        parent=body,
        leftIndent=10,
        bulletIndent=0,
    )
    table_cell = ParagraphStyle(
        "TableCell",
        parent=body,
        fontSize=7.2,
        leading=9,
        spaceAfter=0,
    )
    table_head = ParagraphStyle(
        "TableHead",
        parent=table_cell,
        fontName="ArialCustom-Bold",
    )

    story = []
    lines = SRC.read_text(encoding="utf-8").splitlines()
    i = 0
    while i < len(lines):
        line = lines[i].rstrip()
        if not line:
            story.append(Spacer(1, 2 * mm))
            i += 1
            continue
        if line.startswith("# "):
            story.append(p(line[2:], title))
            i += 1
            continue
        if line.startswith("## "):
            story.append(p(line[3:], h2))
            i += 1
            continue
        if line.startswith("|"):
            data, i = parse_table(lines, i)
            wrapped = [[p(cell, table_head if r == 0 else table_cell) for cell in row] for r, row in enumerate(data)]
            col_widths = [28 * mm, 42 * mm, 32 * mm, 58 * mm, 92 * mm]
            table = Table(wrapped, colWidths=col_widths, repeatRows=1)
            table.setStyle(TableStyle([
                ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#d9e8fb")),
                ("TEXTCOLOR", (0, 0), (-1, 0), colors.black),
                ("GRID", (0, 0), (-1, -1), 0.35, colors.HexColor("#999999")),
                ("VALIGN", (0, 0), (-1, -1), "TOP"),
                ("LEFTPADDING", (0, 0), (-1, -1), 4),
                ("RIGHTPADDING", (0, 0), (-1, -1), 4),
                ("TOPPADDING", (0, 0), (-1, -1), 3),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 3),
                ("ROWBACKGROUNDS", (0, 1), (-1, -1), [colors.white, colors.HexColor("#f7f9fc")]),
            ]))
            story.append(table)
            story.append(Spacer(1, 3 * mm))
            continue
        if line.startswith("- "):
            story.append(Paragraph(clean_inline(line[2:]), bullet, bulletText="-"))
            i += 1
            continue
        story.append(p(line, body))
        i += 1

    doc = SimpleDocTemplate(
        str(OUT),
        pagesize=landscape(A4),
        leftMargin=10 * mm,
        rightMargin=10 * mm,
        topMargin=10 * mm,
        bottomMargin=10 * mm,
    )
    doc.build(story)


if __name__ == "__main__":
    build_pdf()
