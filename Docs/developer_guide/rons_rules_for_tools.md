# Ron's Rules for Tools

"Rules for tools" is an informal set of rules that developers should keep in mind when working on interactive tools for translational clinical research. If you follow them, you will create tools that many people will use.

## The rules

1. **You make it, I break it.**
   Tools need to be robust and function with a variety of workflows, not only the one envisioned by the developer.

2. **Your tool does not exist until it works on my laptop with my data.**
   Until then, it's an unconfirmed rumor.

3. **I am lazy. I do not like to move the mouse or to type.**
   User friendliness:
   - Minimize the number of clicks.
   - Minimize the distance my pointer has to travel.
   - Have good default values for the parameters and I/O, and allow me to override them.

4. **No more than one simple parameter.**
   If I need more than 20 seconds to figure out how to set the parameter, I won't. (Beware the dangers of too many parameters.)

5. **I have ADD. Make your algorithm fast.**
   Performance requirements (ADD = attention deficit disorder):
   - Instantaneous is what I really want — this is where the slider comes in.
   - Under a minute is good.
   - Five minutes are acceptable.
   - Ten minutes or more are not.

## How to make algorithms robust

- Build a case library with 10 or more cases.
- Use half for development, cycling through the cases daily.
- Use the other half for testing.

## Citation

**Author:** Ron Kikinis ([ORCID: 0000-0001-7227-7058](https://orcid.org/0000-0001-7227-7058))

**Origin:** Presented at the IMIC workshop at MICCAI 2014; subsequently maintained on the 3D Slicer wiki.

**Citation:** Kikinis, R. (2014). Ron's Rules for Tools. Zenodo. [https://doi.org/10.5281/zenodo.22707222](https://doi.org/10.5281/zenodo.22707222)

The archived version of this document is available on Zenodo: [https://zenodo.org/records/22707222](https://zenodo.org/records/22707222) (licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)).
