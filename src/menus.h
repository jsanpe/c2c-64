#pragma once

class MenuProvider;

typedef const __FlashStringHelper* fakestr;
typedef void (*callback_fn)(MenuProvider *, byte);


class MenuItem {
    friend class MenuProvider;
    MenuItem *_prev;
    MenuItem *_next;
    MenuProvider *root = NULL;
    public:
    MenuItem *prev(){return _prev;}
    MenuItem *next(){return _next;}
    virtual void setRoot(MenuProvider* root){this->root=root;}
    virtual fakestr getName()=0;
    virtual fakestr getValue(){return NULL;}
    virtual MenuProvider* action(){return root;}
};

class MenuValueItem: public MenuItem {
    public:
    virtual byte getRawValue()=0;
};

class MultiValueMenuItem: public MenuValueItem {
  fakestr name;
  byte levels;
  byte value;
  protected:
  virtual fakestr getValue(byte index)=0;
  virtual MenuProvider* changeEvent()=0;
  public:
  MultiValueMenuItem(fakestr name, byte levels, byte value=0);
  virtual fakestr getName();
  virtual fakestr getValue();
  virtual void setValue(byte value);
  virtual void setLevels(byte levels);
  virtual byte getRawValue();
  virtual MenuProvider* action();
};


template<uint8_t n_levels> class ArrayItem: public MultiValueMenuItem {
  fakestr valueNames[n_levels];
  uint8_t index=0;
  MenuProvider *provider;
  callback_fn f;
  public:
  ArrayItem():MultiValueMenuItem(F(""), n_levels, 0){}
  ArrayItem(MenuProvider *provider, fakestr name, 
    byte value, callback_fn f): MultiValueMenuItem(name, n_levels, value)
  {
    this->provider=provider;
    this->f = f;
  }
  bool addValue(fakestr value) {
    if(index>=n_levels) return false;
    valueNames[index++] = value;
    return true;
  }
  template<typename... Args> bool addValue(fakestr t, Args... args ) {
      addValue(t);
      return addValue(args...);
  }
  virtual fakestr getValue(byte index){
    return valueNames[index];
  }
  MenuProvider* changeEvent() {
      f(provider, this->getRawValue());
      return provider;
  }
};


typedef ArrayItem<2> BinaryMenuItem;
typedef ArrayItem<3> TrinaryMenuItem;

class ArrayMenuItemFactory {
  MenuProvider *provider;
  public:
  ArrayMenuItemFactory(MenuProvider *provider){
    this->provider = provider;
  }

  BinaryMenuItem createDuplet(fakestr name, fakestr  v1, fakestr v2, byte value, callback_fn f) {
    BinaryMenuItem item = BinaryMenuItem(this->provider, name, value, f);
    item.addValue(v1, v2);
    return item;
  }

  TrinaryMenuItem createTriplet(fakestr name, fakestr  v1, fakestr v2, fakestr v3, byte value, callback_fn f) {
    TrinaryMenuItem item = TrinaryMenuItem(this->provider, name, value, f);
    item.addValue(v1, v2, v3);
    return item;
  }
};





class BackMenuItem: public MenuItem {
    public:
    BackMenuItem(MenuProvider *root){
        MenuItem::setRoot(root);
    }
    virtual void setRoot(MenuProvider *root){}
    fakestr getName(){return F(">Back<");}
};

class MenuProvider: public MenuItem {
    MenuItem *head = NULL;
    MenuItem *tail = NULL;
    MenuItem *selected = NULL;
    fakestr name;

    public:
    MenuProvider(const __FlashStringHelper *name) {
        this->name = name;
    }
    virtual void initMenu(MenuProvider *root) {}
    fakestr getName() {return this->name;}
    void addItem(MenuItem *item) {
        if(head == NULL) {
            head = item;
            head->_prev = NULL;
            selected = item;
        } else {
            tail->_next = item;
            item->_prev = tail;
        }
        tail = item;
        item->_next = NULL;
        item->setRoot(this);
    }
    MenuItem *browseReset() {selected=head; return selected;}
    MenuItem *browseUp() {if(selected->_prev)selected=selected->_prev; return selected;}
    MenuItem *browseDown() {if(selected->_next)selected=selected->_next; return selected;}
    virtual MenuItem *getVisibleItem(){return selected;}
    virtual void setRoot(MenuProvider *root) {
        this->addItem(new BackMenuItem(root));
    }
};
