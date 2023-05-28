class Doodad {};
class Doohickey {};
class Gadget {};
class Widget {};

class Thingamabob :
  public Gadget,
  private Doodad,
  public virtual Doohickey,
  private virtual Widget
{};

class Thingamajig :
  public Gadget,
  public virtual Doodad,
  private virtual Doohickey
{};

class Foo : public Thingamajig {};

class Foobar : public Thingamajig, public virtual Doodad {};
