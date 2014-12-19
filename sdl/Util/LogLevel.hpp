









































//FIXME: logLevel(0) -> pure virtual method called









#undef LEVELATLEAST
  return log4cxx::Level::getOff();
}

// defaults

  log4cxx::LevelPtr r=logLevel(i);
  return l.empty() ? r : log4cxx::Level::toLevel(l, r);
}





