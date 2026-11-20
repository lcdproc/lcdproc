# Contribution Guidelines

Thanks for considering contributing to the LCDproc Project! We welcome user and developer participation of all kinds, from simple issue/bug reports and security/CVE discoveries to documentation, bug fixes, new drivers, new clients and fixes/feature additions to the project's core (and included clients).

Whether you're interested in improving LCDproc's behavior by adding features or fixing bugs, improving its documentation (to clarify something we didn't, or to help more users successfully adopt the software), or just helping coordinate work by wrangling bugs, asking questions and proposing fixes (even if you can't/won't implement them yourself), we welcome input from our community.

This project is absurdly informal. You'll find a conversational tone and friendly-by-default demeanor throughout, in the documentation, code, and our handling/use of GitHub's features to interact with the community and manage the project. We're not going to "rules lawyer" your bug reports, code contributions or other nonsense. If you're helping out, we're happy to have you. Instead of demanding you follow "issue reporting templates" and adhere to strict code standards, we lay out some basics in this document and will ask questions (or suggest/request revisions in PRs) when something's unclear or not "to standard."

If you're not sure how to proceed but you'd like to contribute anyway, don't hesitate to open a general issue and ask. We'll be glad to help. The project is composed entirely of volunteers, so (barring security vulnerabilities) we make no promises as to response time, but we do the best we can.

## Bug Reports

You're welcome to create a new issue (in the "Issues") tab to report actual (or even just "suspected") unexpected behavior (crashes, ignored options, `rm -Rf /`, etc.).

There's no "mandatory bug template." If one pops up by default when you create an issue, it's because GitHub required it as part of its "Community Standards" mandates and we've inserted a non-intrusive default you're free to ignore.

Just keep it simple. A good bug report (or support request, or general question, etc.) just provides a few elements:

- A description of the problem -- what's broken? 
- Steps (if relevant) to reproduce the problem -- What are you trying? What did you expect to happen? What happened instead? What should one of us do on our own equipment to test things out so we can see the problem for ourselves?
- Hardware details -- this is a project (mostly) focused on rather esoteric hardware (by today's standards anyway), so please let us know what kind of LCD module and host computer you're working with; even if we don't have that hardware for local testing, it might help us identify the root cause or suggest a fix
- Software details -- what OS are you running? What version? If you've built LCDproc from sources, what toolchain (i.e. compiler suite) did you build it with?

## Pull Requests

Code patches and documentation changes are always welcome if they actually fix something, add a feature, or improve the documentation (for very broadly-defined values of "improve").

- Stick to C99 for the server and included clients
- Minimize dependencies; prefer building a new, standalone client if your changes include a major external dependency
- Avoid breaking changes when possible (without discussion, anyway)
- Test your changes locally as thoroughly as possible; testing on real hardware is mandatory when adding a new device driver
  - Changes we cannot adequately test (within reason -- obviously hardware drivers for equipment we don't have are an exception here) may not be accepted without further discussion
- If providing files or scripts to help package LCDproc for a new distribution or operating system, ensure the resulting package(s) comply with the target platform's publicly-stated standards, requirements and legal obligations (as appropriate)
- Include your preferred attribution in your PR (preferably by updating the AUTHORS.md file directly to show exactly how you'd like to be credited)

## Security Fixes

Refer to `SECURITY.md` for our security policy, including instructions on how to report a vulnerability. In short, we prefer fast and open disclosure as opposed to "backdoor reporting and silence prior to releasing a fix." Light that fire under our feet! The world is a more secure place when vulnerabilities are widely known and fixed quickly. Hiding achieves nothing. We're not securing nuclear launch codes; we're drawing letters and numbers on little screens here.

## Code of Conduct

We follow the [Code of Merit](https://github.com/dpyro/Code-of-Merit), but GitHub mandates it be called `CODE_OF_CONDUCT.md` on their platform. We are _keenly_ aware of the intrusive and subversive nature of the modern "Code of Conduct" concept and are deathly allergic to attempts to inject politics into the project. Don't try. You will be banned without discussion. LCD modules don't care who you vote for or date, and we don't either.