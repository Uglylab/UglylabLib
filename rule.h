#ifndef RULE_H
#define RULE_H

class Rule {
public:
    Rule();

    virtual ~Rule();

    virtual void execute() = 0;  // Each specific rule will implement its own execution logic.
};

#endif // RULE_H
