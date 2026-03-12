import os
import re
import datetime
import argparse

def convert_to_blog_style(source_path, github_link, title, tags, existing_id=None):
    if not os.path.exists(source_path):
        print(f"Error: Source path {source_path} not found.")
        return None

    with open(source_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Remove the first H1 if it's there (we'll use the title in frontmatter)
    content = re.sub(r'^# .*\n', '', content, flags=re.MULTILINE)

    tag_list = "\n".join([f"  - {tag}" for tag in tags])
    
    # Use existing ID if provided, otherwise null
    article_id = f'"{existing_id}"' if existing_id else "null"

    blog_content = f"""---
title: "{title}"
tags:
{tag_list}
private: false
updated_at: '{datetime.datetime.now().isoformat()}'
id: {article_id}
organization_url_name: null
slide: false
ignorePublish: false
---

{content}
"""
    return blog_content

def get_existing_id(output_path):
    if not os.path.exists(output_path):
        return None
    with open(output_path, 'r', encoding='utf-8') as f:
        content = f.read()
    match = re.search(r'^id:\s+["\']?([^\n"\']+)["\']?', content, re.MULTILINE)
    if match and match.group(1) != "null":
        return match.group(1)
    return None

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert a development report to a Qiita blog post.")
    parser.add_argument("--source", default="qiita_development_report.md", help="Path to the source markdown report.")
    parser.add_argument("--output", default="public/blog_post.md", help="Path to the output blog post.")
    parser.add_argument("--title", default="Godot 4 + GDExtension(C++) 開発記", help="Title of the blog post.")
    parser.add_argument("--tags", default="Godot,Cpp,GDExtension,AI", help="Comma-separated tags.")
    parser.add_argument("--github", default="https://github.com/roripika/godot_cxx_2d_game_template/tree/main", help="GitHub repository link.")

    args = parser.parse_args()
    
    tags = [t.strip() for t in args.tags.split(",")]
    
    # Absolute path conversion if needed
    source_abs = os.path.abspath(args.source)
    output_abs = os.path.abspath(args.output)
    
    # Check for existing ID to preserve it
    existing_id = get_existing_id(output_abs)

    blog_md = convert_to_blog_style(source_abs, args.github, args.title, tags, existing_id)
    
    if blog_md:
        os.makedirs(os.path.dirname(output_abs), exist_ok=True)
        with open(output_abs, 'w', encoding='utf-8') as f:
            f.write(blog_md)
        print(f"Blog post generated at: {output_abs} (ID: {existing_id if existing_id else 'new'})")
